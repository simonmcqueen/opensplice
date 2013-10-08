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

public class ReaderMgr<DR extends DataReader> extends EntityMgr {
    private final SubscriberMgr m_subscriber;
    private final TopicDescriptionMgr m_topic_description;
    private final Class<DR> m_class;
    private DR m_reader;

    public ReaderMgr(SubscriberMgr subscriber,
                     TopicDescriptionMgr topic_description,
                     Class<DR> _class) {
        m_subscriber = subscriber;
        m_topic_description = topic_description;
        m_class = _class;
    }

    public void finalize() {        // REVISIT - useful?
        destroy();
    }

    public DR value() {
        return m_reader;
    }

    public TopicDescriptionMgr topic_description() {
        return m_topic_description;
    }

    public void        create(DataReaderQos reader_qos) {
        destroy();
        DataReader reader = m_subscriber.value().create_datareader(m_topic_description.value(),
                                                                   reader_qos,
                                                                   null,
                                                                   ANY_STATUS.value);
        check(reader,
              "FAILED",
              "Subscriber::create_datareader");
        m_reader = m_class.cast(reader);
        m_entity = reader;
    }

    public void        create() {
        create(DATAREADER_QOS_DEFAULT.value);
    }

    public void destroy() {
        if (m_reader != null) {
            int retcode = m_subscriber.value().delete_datareader((DataReader) m_reader);
            check(retcode,
                  "Subscriber::delete_datareader");
            m_reader = null;
            m_entity = null;
        }
    }

    public void check(int retcode,
                      String operation) {
        /* retcode NO_DATA is not really an error, so
         * we ignore it for now */
        if (retcode != RETCODE_NO_DATA.value) {
            DDSError.check(retcode,
                           operation,
                           m_topic_description.topic_name());
        }
    }

    public void check(Object object,
                      String description,
                      String operation) {
        DDSError.check(object,
                       description,
                       operation,
                       m_topic_description.topic_name());
    }
}
