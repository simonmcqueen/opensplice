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

#ifndef __COMMANDHELPERS_H_
#define __COMMANDHELPERS_H_

#include <stdio.h>
#include "touchstone_vendor_abstraction_c.h"

void DDSTouchStone_transmitterDefPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transmitterDef *td);
void DDSTouchStone_transmitterDefScan  (FILE *stream, DDSTouchStone_transmitterDef *td, int *success);
void DDSTouchStone_transmitterQosPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transmitterQos *tq);
void DDSTouchStone_transmitterQosScan  (FILE *stream, DDSTouchStone_transmitterQos *tq, int *success);
void DDSTouchStone_receiverDefPrint    (char *buffer, unsigned int bufferSize, DDSTouchStone_receiverDef *rd);
void DDSTouchStone_receiverDefScan     (FILE *stream, DDSTouchStone_receiverDef *rd, int *success);
void DDSTouchStone_receiverQosPrint    (char *buffer, unsigned int bufferSize, DDSTouchStone_receiverQos *rq);
void DDSTouchStone_receiverQosScan     (FILE *stream, DDSTouchStone_receiverQos *rq, int *success);
void DDSTouchStone_transceiverDefPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transceiverDef *sd);
void DDSTouchStone_transceiverDefScan  (FILE *stream, DDSTouchStone_transceiverDef *sd, int *success);
void DDSTouchStone_transceiverQosPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transceiverQos *sq);
void DDSTouchStone_transceiverQosScan  (FILE *stream, DDSTouchStone_transceiverQos *sq, int *success);
void DDSTouchStone_transponderDefPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transponderDef *xd);
void DDSTouchStone_transponderDefScan  (FILE *stream, DDSTouchStone_transponderDef *xd, int *success);
void DDSTouchStone_transponderQosPrint (char *buffer, unsigned int bufferSize, DDSTouchStone_transponderQos *xq);
void DDSTouchStone_transponderQosScan  (FILE *stream, DDSTouchStone_transponderQos *xq, int *success); 

#endif /* __COMMANDHELPERS_H_ */
