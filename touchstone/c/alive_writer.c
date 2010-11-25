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


#include "alive_writer.h"


struct alive_writer_s {
    unsigned long random_id;
    /* unsigned long last_sequence_number; */
    /* double        start_time; */
    struct alive_writer_s *next;
};


static void
alive_writer_insert(
    DDSTouchStone_throughput_message *msg,
    alive_writer *prev_ptr)
{
    alive_writer new_writer;

    new_writer = (alive_writer)malloc(sizeof(*new_writer));
    if (new_writer != NULL) {
        new_writer->random_id = msg->random_id;
        new_writer->next = *prev_ptr;
        *prev_ptr = new_writer;
    }
}


void
alive_writer_find_or_add(
    DDSTouchStone_throughput_message *msg,
    DDS_boolean *is_new,
    alive_writer *alive_writers_head_ptr)
{
    alive_writer current_writer;
    DDS_boolean found = FALSE;

    current_writer = *alive_writers_head_ptr;
    while ((current_writer != NULL) && !found) {
        found = (current_writer->random_id == msg->random_id);
        current_writer = current_writer->next;
    }

    if (!found) {
        *is_new = TRUE;
        alive_writer_insert(msg, alive_writers_head_ptr);
    } else {
        *is_new = FALSE;
    }
}
    
