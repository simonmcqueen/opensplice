/*
 *  DDSTouchStone: a scenario-driven Open Source benchmarking framework
 *  for evaluating the performance of OMG DDS compliant implementations.
 *
 *  Copyright (C) 2008-2009 PrismTech Ltd.
 *  ddstouchstone@prismtech.com
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License Version 3 dated 29 June 2007, as published by the
 *  Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with DDSTouchStone; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

package ddshelp;

import DDS.*;

public class WriterMgr<DW extends DataWriter> extends EntityMgr {
    private final PublisherMgr m_publisher;
    private final TopicMgr m_topic;
    private final Class<DW> m_class;
    private DW m_writer;

    public WriterMgr(PublisherMgr publisher,
                     TopicMgr topic,
                     Class<DW> _class) {
        m_publisher = publisher;
        m_topic = topic;
        m_class = _class;
    }

    public void finalize() {        // REVISIT - useful?
        destroy();
    }

    public DW value() {
        return m_writer;
    }

    public TopicMgr topic() {
        return m_topic;
    }

    public void create(DataWriterQos writer_qos) {
        destroy();
        DataWriter writer = m_publisher.value().create_datawriter(m_topic.value(),
                                                                  writer_qos,
                                                                  null,
                                                                  ANY_STATUS.value);
        check(writer,
              "FAILED",
              "Publisher::create_datawriter");
        m_writer = m_class.cast(writer);
        m_entity = writer;
    }

    public void create() {
        create(DATAWRITER_QOS_DEFAULT.value);
    }

    public void destroy() {
        if (m_writer != null) {
            int retcode = m_publisher.value().delete_datawriter((DataWriter) m_writer);
            check(retcode,
                  "Publisher::delete_datawriter");
            m_writer = null;
            m_entity = null;
        }
    }

    public void check(int retcode,
                      String operation) {
        /* The return code DDS_RETCODE_TIMEOUT is not an error, so
         * we accept it for now. */
        if (retcode != RETCODE_TIMEOUT.value) {
            DDSError.check(retcode,
                           operation,
                           m_topic.topic_name());
        }
    }

    public void check(Object object,
                      String description,
                      String operation) {
        DDSError.check(object,
                       description,
                       operation,
                       m_topic.topic_name());
    }
}
