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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <math.h>

#include "touchstone_vendor_abstraction_c.h"

#define ERROR(message) \
        printf("Excelerator line %d: %s\n", \
               __LINE__, message)

#define MAX_OUTPUT_LENGTH (10000)
#define MAX_GRAPH_LENGTH (50000)
#define MAX_LINE_LEN (1000)	/* maximal line length supported.     */

/* static char *DDSTouchStoneCommandPartition = "DDSTouchStoneCommands"; */ /* Not used */
static char *DDSTouchStoneReportPartition =  "DDSTouchStoneReports";

static DDS_DomainId_t               ExceleratorDomain      = DDS_OBJECT_NIL;
static DDS_DomainParticipantFactory ExceleratorFactory     = DDS_OBJECT_NIL;
static DDS_DomainParticipant        ExceleratorParticipant = DDS_OBJECT_NIL;
static DDS_Subscriber               ExceleratorSubscriber  = DDS_OBJECT_NIL;

/* Report TypeSupport */
static DDSTouchStone_transmitterReportTypeSupport trs = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportTypeSupport    rrs = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportTypeSupport srs = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportTypeSupport xrs = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportTypeSupport   drs = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportTypeSupport       ers = DDS_OBJECT_NIL;

/* Report Topics */
static DDS_Topic trt = DDS_OBJECT_NIL;
static DDS_Topic rrt = DDS_OBJECT_NIL;
static DDS_Topic srt = DDS_OBJECT_NIL;
static DDS_Topic xrt = DDS_OBJECT_NIL;
static DDS_Topic drt = DDS_OBJECT_NIL;
static DDS_Topic ert = DDS_OBJECT_NIL;

/* Report Readers */
static DDSTouchStone_transmitterReportDataReader trr = DDS_OBJECT_NIL;
static DDSTouchStone_receiverReportDataReader    rrr = DDS_OBJECT_NIL;
static DDSTouchStone_transceiverReportDataReader srr = DDS_OBJECT_NIL;
static DDSTouchStone_transponderReportDataReader xrr = DDS_OBJECT_NIL;
static DDSTouchStone_discoveryReportDataReader   drr = DDS_OBJECT_NIL;
static DDSTouchStone_errorReportDataReader       err = DDS_OBJECT_NIL;

static DDS_StatusCondition trrc = DDS_OBJECT_NIL;
static DDS_StatusCondition rrrc = DDS_OBJECT_NIL;
static DDS_StatusCondition srrc = DDS_OBJECT_NIL;
static DDS_StatusCondition xrrc = DDS_OBJECT_NIL;
static DDS_StatusCondition drrc = DDS_OBJECT_NIL;
static DDS_StatusCondition errc = DDS_OBJECT_NIL;

static int t_end = 0;
static float t_times [1000];
static float t_values [1000];
static float t_min_val = 0;
static float t_max_val = 0;
static float t_min_tim = 0;
static float t_max_tim = 0;

static int l_end = 0;
static float l_times [1000];
static float l_values_min [1000];
static float l_values_max [1000];
static float l_values_avg [1000];
static float l_min_val = 0;
static float l_max_val = 0;
static float l_min_tim = 0;
static float l_max_tim = 0;

static float x_offset=50;
static float y_offset=750;
static float x_size=780;
static float y_size=500;

/* file handling */
static int output_level = 0;
static char* output_prefix;

static int check_file_exists (char* filename)
{
    FILE* file;
    if (file = fopen(filename, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

static void write_to_file (char* filename, char* output)
{
  FILE *file;
  file = fopen(filename,"a+"); /* apend file (add text to a file or create a file if it does not exist.*/
  fprintf(file,"%s",output);
  fclose(file);
}

static void write_graph (char* filename, char* output)
{
  FILE *file;
  file = fopen(filename,"w+"); /* apend file (add text to a file or create a file if it does not exist.*/
  fprintf(file,"%s",output);
  fclose(file);
}

static void write_output (char* output, char* filename, int level)
{
  if ( level == 0)
  {
     write_to_file (filename, output);
  }
  else if (level == 1)
  {
     printf("%s", output);
  }
  else
  {
     write_to_file (filename, output);
     printf("%s", output);
  }
}

static void copy_templates ()
{
    char* file_path_from;	/* path to source file.   */
    char* file_path_to;		/* path to target file.   */
    FILE* f_from;		/* stream of source file. */
    FILE* f_to;			/* stream of target file. */
    char buf[MAX_LINE_LEN+1];   /* input buffer.          */
    char* directory;
    directory = malloc (MAX_OUTPUT_LENGTH);

    snprintf(directory, MAX_OUTPUT_LENGTH, "%s/common_files", output_prefix);
    touchstone_os_mkdir(directory);

    snprintf(file_path_from, MAX_OUTPUT_LENGTH, "/home/cwa/base/touchhtml/touchstone/tools/html_templates/Touchstone_name.jpg");
    snprintf(file_path_to, MAX_OUTPUT_LENGTH, "Touchstone_name.jpg");

    printf ("From: %s", file_path_from);
    printf ("To: %s", file_path_to);

    /* open the source and the target files. */
    f_from = fopen(file_path_from, "rb");
    if (!f_from) {
	fprintf(stderr, "Cannot open source file: ");
	perror("");
	exit(1);
    }
    f_to = fopen(file_path_to, "wb");
    if (!f_from) {
	fprintf(stderr, "Cannot open target file: ");
	perror("");
	exit(1);
    }

    /* copy source file to target file, line by line. */
    while (fgets(buf, MAX_LINE_LEN+1, f_from)) {
	if (fputs(buf, f_to) == EOF) {  /* error writing data */
	    fprintf(stderr, "Error writing to target file: ");
	    perror("");
	    exit(1);
	}
    }
    if (!feof(f_from)) { /* fgets failed _not_ due to encountering EOF */
        fprintf(stderr, "Error reading from source file: ");
        perror("");
        exit(1);
    }

    /* close source and target file streams. */
    if (fclose(f_from) == EOF) {
	fprintf(stderr, "Error when closing source file: ");
	perror("");
    }
    if (fclose(f_to) == EOF) {
	fprintf(stderr, "Error when closing target file: ");
	perror("");
    }
    free (directory);
}

static void copy_templates2 ()
{
  char c[100];
  FILE *inFile;
  FILE *outFile;
  char sourceFile;
  char destFile;
  int Byte;
  int i;


  printf("Enter the File Name to read: ");
  scanf("%s",&sourceFile);
  printf("Enter the File Name to write to: ");
  scanf("%s",&destFile);

  inFile = fopen("text", "rb");
  printf("open source file");
  /*open a text file for reading in binary */
  outFile = fopen(&destFile, "wb");
  /*open a text file for writing in binary*/
  printf("open dest file");

    printf("File Opened Successfully.");
    while(1)
    {
      if(Byte!=EOF)
      {
	Byte=fgetc(inFile);
	fputc(Byte,outFile);
      }
      else
      {
        break;
      }
    }
}

/* html generation */

static char* format_table (char* contents, char* css_class)
{
  char* output;
  output = malloc (MAX_OUTPUT_LENGTH);
  snprintf(output, MAX_OUTPUT_LENGTH, "<table class='%s' cellpadding=3>%s</table>", css_class, contents);
  return output;
}

static void render_graph (char* filename, char* type, int app, int part, int comp, char* report_link)
{
  char* output;
  char* head;
  char* tail;
  char* axis;
  char* markers;
  char* points;
  char* point_name;
  int i;
  int p_time;
  int p_value;
  int p_value_min;
  int p_value_max;

  float g_max_val;
  float g_min_val;
  float g_max_tim;
  float g_min_tim;
  int g_end = 0;
  float g_times [1000];
  float g_values [1000];

  output = malloc (MAX_GRAPH_LENGTH);
  head = malloc (MAX_OUTPUT_LENGTH);
  tail = malloc (MAX_OUTPUT_LENGTH);
  axis = malloc (MAX_OUTPUT_LENGTH);
  markers = malloc (MAX_OUTPUT_LENGTH);

  points = malloc (MAX_OUTPUT_LENGTH);
  point_name = malloc (MAX_OUTPUT_LENGTH);

if (type == "receiver")
{
  g_max_val = t_max_val;
  g_min_val = t_min_val;
  g_max_tim = t_max_tim;
  g_min_tim = t_min_tim;
  g_end = t_end;
}
else
{
  g_max_val = l_max_val;
  g_min_val = l_min_val;
  g_max_tim = l_max_tim;
  g_min_tim = l_min_tim;
  g_end = l_end;
}

  snprintf(head, MAX_OUTPUT_LENGTH, "<html><head><link rel=stylesheet href='../%s_css.css' TYPE='text/css'></head><body><div class='report_title'>%s %d reports</div><div class='component_id'><table class='ids' cellpadding=5><tr><td class='vgrid'>application id</td><td class='vgrid'>partition id</td><td class='vgrid'>%s id</td></tr><tr><td class='vgrid'>%d</td><td class='vgrid'>%d</td><td class='vgrid'>%d</td></tr></table></div><div class='view_link'><a href='%s' target='main'>Switch to Report View</a></div><div class='graph_spacer'>Summary Graph:</div>", output_prefix, type, comp, type, app, part, type, report_link);

  write_graph (filename, head);

if (type == "receiver")
{

  snprintf(axis, MAX_OUTPUT_LENGTH, "<div class='x-label' style='position:absolute;top:750;left:400;width:40;text-align:center'>Timestamp<br>(secs)</div><div class='y-label' style='position:absolute;top:350;left:20;width:20'>T<br>h<br>r<br>o<br>u<br>g<br>h<br>p<br>u<br>t</div><div class='axis' style='position:absolute;left:50;top:150;height:600;width:800'><IMG SRC='bluespacer.gif' WIDTH=2 HEIGHT=100% border=0><IMG SRC='bluespacer.gif' WIDTH=100% HEIGHT=2 BORDER=0>");

}
else
{

  snprintf(axis, MAX_OUTPUT_LENGTH, "<div class='x-label' style='position:absolute;top:750;left:400;width:40;text-align:center'>Timestamp<br>(secs)</div><div class='y-label' style='position:absolute;top:350;left:20;width:20'>L<br>a<br>t<br>e<br>n<br>c<br>y</div><div class='axis' style='position:absolute;left:50;top:150;height:600;width:800'><IMG SRC='bluespacer.gif' WIDTH=2 HEIGHT=100% border=0><IMG SRC='bluespacer.gif' WIDTH=100% HEIGHT=2 BORDER=0>");

}

  write_to_file (filename, axis);

if (type == "receiver")
{
  for (i = 0; i < g_end; i++)
  {

    p_value = (int)(((t_values[i]/g_max_val)*y_size)-5);
    p_time = (int)(((t_times[i]/g_max_tim)*x_size)-5);

    snprintf(point_name, MAX_OUTPUT_LENGTH, "point%d", i);
    snprintf(points, MAX_OUTPUT_LENGTH, "<div class='%s' style='position:absolute;bottom:%d;left:%d'><IMG SRC='bblue.gif' title='T.Stamp: %d sec| Throughput: %d sam/sec'></div>", point_name, p_value, p_time, (int)t_times[i], (int)t_values[i]);
    write_to_file (filename, points);

    //printf ("\nthroughput \nvalue = %f / time = %f \n p_value = %d / p_time = %d\n", t_values[i], t_times[i], p_value, p_time);
  }
}
else
{
  for (i = 0; i < g_end; i++)
  {

    p_value = (int)(((l_values_avg[i]/g_max_val)*y_size)-5);
    p_value_min = (int)(((l_values_min[i]/g_max_val)*y_size)-5);
    p_value_max = (int)(((l_values_max[i]/g_max_val)*y_size)-5);

    p_time = (int)(((l_times[i]/g_max_tim)*x_size)-5);

    snprintf(point_name, MAX_OUTPUT_LENGTH, "point%d", i);
    snprintf(points, MAX_OUTPUT_LENGTH, "<div class='%s_avg' style='position:absolute;bottom:%d;left:%d'><IMG SRC='bblue.gif' title='T.Stamp: %d sec| Average Latency: %d sec'></div><div class='%s_max' style='position:absolute;bottom:%d;left:%d'><IMG SRC='bred.gif' title='T.Stamp: %d sec| Maximum Latency: %d sec'></div><div class='%s_min' style='position:absolute;bottom:%d;left:%d'><IMG SRC='bgreen.gif' title='T.Stamp: %d sec| Minimum Latency: %d sec'></div>", point_name, p_value, p_time, (int)l_times[i], (int)l_values_avg[i], point_name, p_value_max, p_time, (int)l_times[i], (int)l_values_max[i], point_name, p_value_min, p_time, (int)l_times[i], (int)l_values_min[i]);
    write_to_file (filename, points);

    //printf ("\nlatency\nvalue = %f / time = %f \n p_value = %d / p_time = %d\n", l_values_avg[i], l_times[i], p_value, p_time);

  }
}

  snprintf(markers, MAX_OUTPUT_LENGTH, "</div><div class='y-max' style='position:absolute;top:%d;left:20'; width:20>%d</div><div class='y-min' style='position:absolute;top:%d;left:%d'>0</div><div class='x-max' style='position:absolute;top:750;left:%d'>%d</div>", (int)((y_offset-y_size)-10), (int)g_max_val, (int)(y_offset), (int)(x_offset-10), (int)(x_size+x_offset), (int)g_max_tim);
  write_to_file (filename, markers);

  snprintf(tail, MAX_OUTPUT_LENGTH, "</body></html>");
  write_to_file (filename, tail);

  free (output);
  free (head);
  free (tail);
  free (axis);
  free (markers);
  free (points);
  free (point_name);
}


static void report_line_break (char* filename)
{
  write_to_file (filename, "<br>");
}

static char* format_css_table (char* contents, int border, char* class)
{
  char* output;
  output = malloc (MAX_OUTPUT_LENGTH);
  snprintf(output, MAX_OUTPUT_LENGTH, "<div class=%s><table border=%d cellpadding=3>%s</table></div>", class, border, contents);
  return output;
}

static void create_css_file ()
{
  char* filename;
  filename = malloc (MAX_OUTPUT_LENGTH);

  snprintf(filename, MAX_OUTPUT_LENGTH, "%s/%s_css.css", output_prefix, output_prefix);
/*
  write_to_file(filename, "table.transmitter{left:5}\n");
  write_to_file(filename, "table.receiver{left:415}\n");
  write_to_file(filename, "table.transceiver{left:625}\n");
  write_to_file(filename, "table.transponder{left:835}\n");

  write_to_file(filename, "div.reports{position:relative}\n");
  write_to_file(filename, "div.transmitter{position:static;top:0;left:0;width:200px}\n");
  write_to_file(filename, "div.receiver{position:absolute;left:0;width:200px}\n");
  write_to_file(filename, "div.transceiver{position:absolute;left:460;width:200px}\n");
  write_to_file(filename, "div.transponder{position:relative;top:0;left:690;width:200px}\n");
*/

  write_to_file(filename, "div.header{position:absolute;top:5;left:5;height:150}\n");
  write_to_file(filename, "div.logo{position:absolute;top:5;left:550;width:250;height:250}\n");
  write_to_file(filename, "div.selectbox{position:absolute;top:180;left:5;height:50}\n");
  write_to_file(filename, "div.menuframe{position:absolute;top:250;left:5;width:150;height:800}\n");
  write_to_file(filename, "div.mainframe{position:absolute;top:250;left:165;width:800;height:800}\n");
  write_to_file(filename, "img.logoimage{width:250;height:250}\n");

  write_to_file(filename, "div.report_title{position:absolute;top:10;left:10}\n");
  write_to_file(filename, "div.view_link{position:absolute;top:70;left:10}\n");
  write_to_file(filename, "div.component_id{position:absolute;top:10;left:300}\n");
  write_to_file(filename, "div.report_spacer{position:relative;top:100;left:10;height:150;width:400;}\n");
  write_to_file(filename, "div.graph_spacer{position:relative;top:100;left:10;height:50;width:600;}\n");


  write_to_file(filename, "table.ids{border-style:solid;border-width:thin;border-collapse:collapse;margin:5}\n");
  write_to_file(filename, "TD.vgrid{border:solid;border-width:thin}\n");
  write_to_file(filename, "table.report{border-style:solid;border-width:thin;border-collapse:collapse}\n");
  write_to_file(filename, "table.nodec{border-style:none;border-collapse:collapse}\n");
  write_to_file(filename, "table.index{border-style:outset;width:135;background-color:orange}\n");
/*
  write_to_file(filename, "div.graph{position:absolute;height:600;width:800;left:5;top:150;}\n");
  write_to_file(filename, "div.axis{position:absolute;left:50;top:150;height:600;width:800;}\n");
  write_to_file(filename, "div.y-label{position:absolute;top:350;left:20;width:20;}\n");
  write_to_file(filename, "div.x-label{position:absolute;top:750;left:400;width:80;}\n");
*/
  free (filename);
}

static void create_component_index (char* type)
{
  char* output;
  char* filename;
  char* directory;

  output = malloc (MAX_OUTPUT_LENGTH);
  filename = malloc (MAX_OUTPUT_LENGTH);
  directory = malloc (MAX_OUTPUT_LENGTH);

  snprintf(directory, MAX_OUTPUT_LENGTH, "%s/%s", output_prefix, type);
  touchstone_os_mkdir(directory);

  snprintf(filename, MAX_OUTPUT_LENGTH, "%s/%s/%s_%s_index.html", output_prefix, type, output_prefix, type);
  snprintf(output, MAX_OUTPUT_LENGTH, "<link rel=stylesheet href='../%s_css.css' TYPE='text/css'>", output_prefix);

  write_to_file (filename, output);

  free (directory);
  free (filename);
  free (output);

}

static void create_index ()
{
  char* output;
  char* filename;

  output = malloc (MAX_OUTPUT_LENGTH);
  filename = malloc (MAX_OUTPUT_LENGTH);

  touchstone_os_mkdir(output_prefix);

  create_component_index ("transmitter");
  create_component_index ("receiver");
  create_component_index ("transceiver");
  create_component_index ("transponder");

  snprintf(filename, MAX_OUTPUT_LENGTH, "%s/%s_index.html", output_prefix, output_prefix);

  snprintf(output, MAX_OUTPUT_LENGTH, "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0 Transitional//EN'><html><head><link rel=stylesheet href='%s_css.css' TYPE='text/css'><title>%s Watcher Index for DDSTouchStone</title></head><body><div class='header'><h1>%s watcher index</h1></div><div class='logo'><img class='logoimage' src='Touchstone_name.jpg'></div><div class='selectbox'>Reports from %s components:<br><form name='form1'><select name='menu'><option value='transmitter/%s_transmitter_index.html'>transmitters</option><option value='receiver/%s_receiver_index.html'>receivers</option><option value='transceiver/%s_transceiver_index.html'>transceivers</option><option value='transponder/%s_transponder_index.html'>transponders</option></select><input type='button' onClick='parent.menu.location.href=document.form1.menu.options[document.form1.menu.selectedIndex].value;' value='display'></form></div><div class='menuframe'><iframe name='menu' width=150 height=800></iframe></div><div class='mainframe'><iframe name='main' width=900 height=800></iframe></div></body></html>", output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix);

/*
  snprintf(output, MAX_OUTPUT_LENGTH, "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0 Transitional//EN'><html><head><link rel=stylesheet href='%s/%s_css.css' TYPE='text/css'><title>%s DDSTouchStone Report Index</title><h1>%s DDSTouchStone Report Index</h1></head><body>Reports from %s components:<br><form name='form1'><select name='menu'><option value='transmitter/%s_transmitter_index.html'>transmitters</option><option value='receiver/%s_receiver_index.html'>receivers</option><option value='transceiver/%s_transceiver_index.html'>transceivers</option><option value='transponder/%s_transponder_index.html'>transponders</option></select><input type='button' onClick='parent.menu.location.href=document.form1.menu.options[document.form1.menu.selectedIndex].value;' value='display reports'></form><iframe name='menu' width=200 height=600></iframe><iframe name='main' width=800 height=600></iframe></body></html>", output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix);

*/

/*
  snprintf(output, MAX_OUTPUT_LENGTH, "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.0 Transitional//EN'><html><head><title>%s DDSTouchStone Report Index</title><h1>%s DDSTouchStone Report Index</h1></head><body>Report pages from %s components:<br><br><b><a href='transmitter/%s_transmitter_index.html' target='menu'>Transmitter reports</a><br><a href='receiver/%s_receiver_index.html' target='menu'>Receiver reports</a><br><a href='transceiver/%s_transceiver_index.html' target='menu'>Transceiver reports</a><br><a href='transponder/%s_transponder_index.html' target='menu'>Transponder reports</a></b> <br><br><iframe name='menu' width=200 height=600></iframe><iframe name='main' width=800 height=600></iframe></body></html>", output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix, output_prefix);
*/
  write_to_file (filename, output);

  free (filename);
  free (output);
}

static void add_to_index (char* link, char* type, int app, int part, int comp)
{
  char* output;
  char* filename;

  output = malloc (MAX_OUTPUT_LENGTH);
  filename = malloc (MAX_OUTPUT_LENGTH);

  snprintf(filename, MAX_OUTPUT_LENGTH, "%s/%s/%s_%s_index.html", output_prefix, type, output_prefix, type);

  snprintf(output, MAX_OUTPUT_LENGTH, "<tr><td><a href='%s.html' target='main'><b>%s %d</b></a></td></tr><tr><td>application id: %d <br> partition id: %d</td></tr>", link, type, comp, app, part);

  write_to_file (filename, format_table(output, "index"));
  report_line_break (filename);

  free (filename);
  free (output);
}

static void create_report_file (char* filename, char* type, int app, int part, int comp)
{
  char* output;
  output = malloc (MAX_OUTPUT_LENGTH);

  snprintf(output, MAX_OUTPUT_LENGTH, "<link rel=stylesheet href='../%s_css.css' TYPE='text/css'><h2>%s %d reports</h2><table class='ids' cellpadding=5><tr><td class='vgrid'>application id</td><td class='vgrid'>partition id</td><td class='vgrid'>%s id</td></tr><tr><td class='vgrid'>%d</td><td class='vgrid'>%d</td><td class='vgrid'>%d</td></tr></table><h3>Reports Posted:</h3>", output_prefix, type, comp, type, app, part, type);

  write_to_file (filename, output);

  free (output);
}

static void create_report_file2 (char* filename, char* type, int app, int part, int comp, char* graph_link)
{
  char* output;
  output = malloc (MAX_OUTPUT_LENGTH);

  snprintf(output, MAX_OUTPUT_LENGTH, "<link rel=stylesheet href='../%s_css.css' TYPE='text/css'><div class='report_title'>%s %d reports</div><div class='component_id'><table class='ids' cellpadding=5><tr><td class='vgrid'>application id</td><td class='vgrid'>partition id</td><td class='vgrid'>%s id</td></tr><tr><td class='vgrid'>%d</td><td class='vgrid'>%d</td><td class='vgrid'>%d</td></tr></table></div><div class='view_link'><a href='%s' target='main'>Switch to Graph View</a></div><div class='report_spacer'>Reports Posted:</div>", output_prefix, type, comp, type, app, part, type, graph_link);

  write_to_file (filename, output);

  free (output);
}

static DDSTouchStone_timestamp
get_timestamp_from_dds_time(
    DDS_Time_t *time)
{
    DDSTouchStone_timestamp timestamp;

    timestamp = time->sec * 1000000 +
                time->nanosec / 1000;
    return timestamp;
}

static void
report_reader_status (
    DDSTouchStone_readerStatus *status)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH, "%d,%d,%d",
           status->samples_lost,
           status->samples_rejected,
           status->deadlines_missed);
    //write_output (output, output_level);
}

static void
report_writer_status (
    DDSTouchStone_writerStatus *status)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "%d", status->deadlines_missed);
    //write_output (output, output_level);
}

static char* add_metric_headers ()
{
  return "<tr><td class='vgrid'></td><td class='vgrid'>count</td><td class='vgrid'>min</td><td class='vgrid'>avg</td><td class='vgrid'>max</td><td class='vgrid'>dev</td></tr>";
}

static char*
report_metrics (
    DDSTouchStone_metricsReport *metrics)
{
    char* output;
    output = malloc (MAX_OUTPUT_LENGTH);

    snprintf(output, MAX_OUTPUT_LENGTH,  "<td class='vgrid'>%3.1f%%</td><td class='vgrid'>%d</td><td class='vgrid'>%.0f</td><td class='vgrid'>%.0f</td><td class='vgrid'>%.0f</td><td class='vgrid'>%.2f</td>",
           metrics->percentile,
           metrics->sample_count,
           metrics->minimum,
           metrics->average,
           metrics->maximum,
           metrics->deviation);
    return output;
}

static void
process_transmitterReport (
    DDSTouchStone_transmitterReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Transmitter,%d,%d,%d,%.0f,%.0f",
           report->application_id,
           report->partition_id,
           report->transmitter_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time));
    //write_output (output, output_level);
    report_writer_status(&report->writer_status);
    //write_output ("\n", output_level);
}

static void
process_receiverReport (
    DDSTouchStone_receiverReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    char* output2;
    char filename[MAX_OUTPUT_LENGTH];
    char link[MAX_OUTPUT_LENGTH];
    char graph_filename[MAX_OUTPUT_LENGTH];
    char graph_link[MAX_OUTPUT_LENGTH];
    char report_link[MAX_OUTPUT_LENGTH];

    snprintf(link, MAX_OUTPUT_LENGTH, "%s_receiver_%d_%d_%d", output_prefix, report->application_id, report->partition_id, report->receiver_id);

    snprintf(filename, MAX_OUTPUT_LENGTH, "%s/receiver/%s.html", output_prefix, link);
    snprintf(graph_filename, MAX_OUTPUT_LENGTH, "%s/receiver/%s_graph.html", output_prefix, link);
    snprintf(graph_link, MAX_OUTPUT_LENGTH, "%s_graph.html", link);
    snprintf(report_link, MAX_OUTPUT_LENGTH, "%s.html", link);

    if (check_file_exists(filename) == 0)
    {
       create_report_file2 (filename, "receiver", report->application_id, report->partition_id, report->receiver_id, graph_link);
       add_to_index (link, "receiver", report->application_id, report->partition_id, report->receiver_id);
    }


    if (t_end == 0)
    {
      t_min_val = report->read_msgs_per_second;
    }
    else if (report->read_msgs_per_second < t_min_val)
    {
      t_min_val = report->read_msgs_per_second;
    }

    if (t_max_val < report->read_msgs_per_second)
    {
      t_max_val = report->read_msgs_per_second;
    }


    if (t_end == 0)
    {
      t_min_tim = get_timestamp_from_dds_time(source_time);
    }
    t_max_tim = ((get_timestamp_from_dds_time(source_time) - t_min_tim)/1000000)+20;

    t_values[t_end] = report->read_msgs_per_second;
    t_times[t_end] = (get_timestamp_from_dds_time(source_time) - t_min_tim)/1000000;
    t_end++;

    render_graph (graph_filename, "receiver", report->application_id, report->partition_id, report->receiver_id, report_link);

    snprintf(output, MAX_OUTPUT_LENGTH,
		"<tr><td class='vgrid'>source time</td><td class='vgrid'>%.0f</td><td class='vgrid'>Throughput</td><td class='vgrid'>%d bytes/sec</td></tr><tr><td class='vgrid'>arrival time</td><td class='vgrid'>%.0f</td><td class='vgrid'>Read rate</td><td class='vgrid'>%d samples/sec</td></tr>",
           get_timestamp_from_dds_time(source_time),
           report->read_bytes_per_second,
           get_timestamp_from_dds_time(arrival_time),
           report->read_msgs_per_second);

//    write_output2 (format_table(output, 1), filename, output_level);
    output2 = format_table(output, "report");
    write_output (output2, filename, output_level);
    free (output2);

    report_reader_status(&report->reader_status);
    report_line_break (filename);
}

static void
process_transceiverReport (
    DDSTouchStone_transceiverReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    char* output2;
    char filename[MAX_OUTPUT_LENGTH];
    char link[MAX_OUTPUT_LENGTH];
    char graph_filename[MAX_OUTPUT_LENGTH];
    char graph_link[MAX_OUTPUT_LENGTH];
    char report_link[MAX_OUTPUT_LENGTH];

	DDSTouchStone_timestamp st,at;

    st = get_timestamp_from_dds_time(source_time);
    at = get_timestamp_from_dds_time(arrival_time);

    snprintf(link, MAX_OUTPUT_LENGTH, "%s_transceiver_%d_%d_%d", output_prefix, report->application_id, report->partition_id, report->transceiver_id);

    snprintf(filename, MAX_OUTPUT_LENGTH, "%s/transceiver/%s.html", output_prefix, link);

    snprintf(graph_filename, MAX_OUTPUT_LENGTH, "%s/transceiver/%s_graph.html", output_prefix, link);
    snprintf(graph_link, MAX_OUTPUT_LENGTH, "%s_graph.html", link);
    snprintf(report_link, MAX_OUTPUT_LENGTH, "%s.html", link);


    if (check_file_exists(filename) == 0)
    {
       create_report_file2 (filename, "transceiver", report->application_id, report->partition_id, report->transceiver_id, graph_link);
       add_to_index (link, "transceiver", report->application_id, report->partition_id, report->transceiver_id);
    }

    DDSTouchStone_metricsReport *l_metrics;

    l_metrics = &report->trip_latency[0];

    if (l_end == 0)
    {
      l_min_val = l_metrics->minimum;
    }
    else if (l_metrics->minimum < l_min_val)
    {
      l_min_val = l_metrics->minimum;
    }

    if (l_max_val < l_metrics->maximum)
    {
      l_max_val = l_metrics->maximum;
    }


    if (l_end == 0)
    {
      l_min_tim = get_timestamp_from_dds_time(source_time);
    }
    l_max_tim = ((get_timestamp_from_dds_time(source_time) - l_min_tim)/1000000)+20;

    l_values_avg[l_end] = l_metrics->average;
    l_values_min[l_end] = l_metrics->minimum;
    l_values_max[l_end] = l_metrics->maximum;

    l_times[l_end] = (get_timestamp_from_dds_time(source_time) - l_min_tim)/1000000;
    l_end++;

    render_graph (graph_filename, "transceiver", report->application_id, report->partition_id, report->transceiver_id, report_link);


    write_to_file (filename, "<table class='nodec' cellpadding=8><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send End2End Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_latency[0]),
		   report_metrics(&report->send_latency[1]),
		   report_metrics(&report->send_latency[2]),
		   report_metrics(&report->send_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Source Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_source_latency[0]),
		   report_metrics(&report->send_source_latency[1]),
		   report_metrics(&report->send_source_latency[2]),
		   report_metrics(&report->send_source_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Delivery Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_trip_latency[0]),
		   report_metrics(&report->send_trip_latency[1]),
		   report_metrics(&report->send_trip_latency[2]),
		   report_metrics(&report->send_trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Arrival Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_arrival_latency[0]),
		   report_metrics(&report->send_arrival_latency[1]),
		   report_metrics(&report->send_arrival_latency[2]),
		   report_metrics(&report->send_arrival_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo End2End Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_latency[0]),
		   report_metrics(&report->echo_latency[1]),
		   report_metrics(&report->echo_latency[2]),
		   report_metrics(&report->echo_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Source Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_source_latency[0]),
		   report_metrics(&report->echo_source_latency[1]),
		   report_metrics(&report->echo_source_latency[2]),
		   report_metrics(&report->echo_source_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Delivery Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_trip_latency[0]),
		   report_metrics(&report->echo_trip_latency[1]),
		   report_metrics(&report->echo_trip_latency[2]),
		   report_metrics(&report->echo_trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Arrival Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_arrival_latency[0]),
		   report_metrics(&report->echo_arrival_latency[1]),
		   report_metrics(&report->echo_arrival_latency[2]),
		   report_metrics(&report->echo_arrival_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Trip Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->trip_latency[0]),
		   report_metrics(&report->trip_latency[1]),
		   report_metrics(&report->trip_latency[2]),
		   report_metrics(&report->trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Inter Arival Time</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->inter_arrival_time[0]),
		   report_metrics(&report->inter_arrival_time[1]),
		   report_metrics(&report->inter_arrival_time[2]),
		   report_metrics(&report->inter_arrival_time[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr></table>");
    report_line_break (filename);

}
/*

static void
process_transceiverReport (
    DDSTouchStone_transceiverReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    char* output2;
    char filename[MAX_OUTPUT_LENGTH];
    char link[MAX_OUTPUT_LENGTH];

	DDSTouchStone_timestamp st,at;

    st = get_timestamp_from_dds_time(source_time);
    at = get_timestamp_from_dds_time(arrival_time);


    snprintf(link, MAX_OUTPUT_LENGTH, "%s_transceiver_%d_%d_%d", output_prefix, report->application_id, report->partition_id, report->transceiver_id);

    snprintf(filename, MAX_OUTPUT_LENGTH, "%s/transceiver/%s.html", output_prefix, link);

    if (check_file_exists(filename) == 0)
    {
       create_report_file (filename, "transceiver", report->application_id, report->partition_id, report->transceiver_id);
       add_to_index (link, "transceiver", report->application_id, report->partition_id, report->transceiver_id);
    }

    write_to_file (filename, "<table class='nodec' cellpadding=8><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send End2End Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_latency[0]),
		   report_metrics(&report->send_latency[1]),
		   report_metrics(&report->send_latency[2]),
		   report_metrics(&report->send_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Source Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_source_latency[0]),
		   report_metrics(&report->send_source_latency[1]),
		   report_metrics(&report->send_source_latency[2]),
		   report_metrics(&report->send_source_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Delivery Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_trip_latency[0]),
		   report_metrics(&report->send_trip_latency[1]),
		   report_metrics(&report->send_trip_latency[2]),
		   report_metrics(&report->send_trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Send Arrival Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->send_arrival_latency[0]),
		   report_metrics(&report->send_arrival_latency[1]),
		   report_metrics(&report->send_arrival_latency[2]),
		   report_metrics(&report->send_arrival_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo End2End Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_latency[0]),
		   report_metrics(&report->echo_latency[1]),
		   report_metrics(&report->echo_latency[2]),
		   report_metrics(&report->echo_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Source Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_source_latency[0]),
		   report_metrics(&report->echo_source_latency[1]),
		   report_metrics(&report->echo_source_latency[2]),
		   report_metrics(&report->echo_source_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Delivery Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_trip_latency[0]),
		   report_metrics(&report->echo_trip_latency[1]),
		   report_metrics(&report->echo_trip_latency[2]),
		   report_metrics(&report->echo_trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Echo Arrival Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->echo_arrival_latency[0]),
		   report_metrics(&report->echo_arrival_latency[1]),
		   report_metrics(&report->echo_arrival_latency[2]),
		   report_metrics(&report->echo_arrival_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr><tr><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Trip Latency</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->trip_latency[0]),
		   report_metrics(&report->trip_latency[1]),
		   report_metrics(&report->trip_latency[2]),
		   report_metrics(&report->trip_latency[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td><td align=left>");

    snprintf(output, MAX_OUTPUT_LENGTH,  "<tr><td colspan=6>Transceiver Inter Arival Time</td></tr>%s<tr>%s</tr><tr>%s</tr><tr>%s</tr><tr>%s</tr>",
		   add_metric_headers (),
		   report_metrics(&report->inter_arrival_time[0]),
		   report_metrics(&report->inter_arrival_time[1]),
		   report_metrics(&report->inter_arrival_time[2]),
		   report_metrics(&report->inter_arrival_time[3]));
    output2 = format_table(output, "nodec");
    write_output (output2, filename, output_level);
    free (output2);
    write_to_file (filename, "</td></tr></table>");
    report_line_break (filename);

}
*/
static void
process_transponderReport (
    DDSTouchStone_transponderReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Transponder,%d,%d,%d,%.0f,%.0f,",
           report->application_id,
           report->partition_id,
           report->transponder_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time));
    //write_output (output, output_level);

    report_writer_status(&report->writer_status);
    report_reader_status(&report->reader_status);
    //write_output ("\n", output_level);
}

static void
process_errorReport (
    DDSTouchStone_errorReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    snprintf(output, MAX_OUTPUT_LENGTH,  "Error,%d,%d,%d,%.0f,%.0f,%s\n",
           report->application_id,
           report->partition_id,
           report->entity_id,
           get_timestamp_from_dds_time(source_time),
           get_timestamp_from_dds_time(arrival_time),
           report->message);
    //write_output (output, output_level);
}

static void
process_discoveryReport (
    DDSTouchStone_discoveryReport *report,
    DDS_Time_t *source_time,
    DDS_Time_t *arrival_time)
{
    char output[MAX_OUTPUT_LENGTH];
    switch (report->report_kind) {
    case DDSTouchStone_DataWriterDiscovery:
        snprintf(output, MAX_OUTPUT_LENGTH,  "Discovery DataWriter,%d,%d,%d,%.0f,%.0f,%.0f,%.0f,%d\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               get_timestamp_from_dds_time(source_time),
               get_timestamp_from_dds_time(arrival_time),
               report->creation_duration,
               report->discovery_time,
               report->samples_missed);
        //write_output (output, output_level);
    break;
    case DDSTouchStone_DataReaderDiscovery:
        snprintf(output, MAX_OUTPUT_LENGTH,  "Discovery DataReader,%d,%d,%d,%.0f,%.0f,%.0f,%.0f\n",
               report->application_id,
               report->partition_id,
               report->entity_id,
               get_timestamp_from_dds_time(source_time),
               get_timestamp_from_dds_time(arrival_time),
               report->creation_duration,
               report->discovery_time);
        //write_output (output, output_level);
    break;
    }
}

static DDS_boolean
process_command (
    DDS_StatusCondition condition)
{
    DDS_sequence_DDSTouchStone_transmitterReport trl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_receiverReport    rrl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transceiverReport srl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_transponderReport xrl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_discoveryReport   drl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_sequence_DDSTouchStone_errorReport       erl = {0,0,DDS_OBJECT_NIL,FALSE};
    DDS_SampleInfoSeq infoList = { 0, 0, NULL, FALSE };
    DDS_ReturnCode_t  result;
    int length, i;

    if (condition == trrc) {
        result = DDSTouchStone_transmitterReportDataReader_read_w_condition (
                     trr, &trl, &infoList, 1, trrc);
        if (result == DDS_RETCODE_OK) {
            length = trl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transmitterReport(
                        &trl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transmitterReportDataReader_return_loan (
                             trr, &trl, &infoList);
            }
        }
    } else if (condition == rrrc) {
        result = DDSTouchStone_receiverReportDataReader_read_w_condition (
                     rrr, &rrl, &infoList, 1, rrrc);
        if (result == DDS_RETCODE_OK) {
            length = rrl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_receiverReport(
                        &rrl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_receiverReportDataReader_return_loan (
                             rrr, &rrl, &infoList);
            }
        }
    } else if (condition == srrc) {
        result = DDSTouchStone_transceiverReportDataReader_read_w_condition (
                     srr, &srl, &infoList, 1, srrc);
        if (result == DDS_RETCODE_OK) {
            length = srl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transceiverReport(
                        &srl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transceiverReportDataReader_return_loan (
                             srr, &srl, &infoList);
            }
        }
    } else if (condition == xrrc) {
        result = DDSTouchStone_transponderReportDataReader_read_w_condition (
                     xrr, &xrl, &infoList, 1, xrrc);
        if (result == DDS_RETCODE_OK) {
            length = xrl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_transponderReport(
                        &xrl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_transponderReportDataReader_return_loan (
                             xrr, &xrl, &infoList);
            }
        }
    } else if (condition == drrc) {
        result = DDSTouchStone_discoveryReportDataReader_read_w_condition (
                     drr, &drl, &infoList, 1, drrc);
        if (result == DDS_RETCODE_OK) {
            length = drl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_discoveryReport(
                        &drl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_discoveryReportDataReader_return_loan (
                             drr, &drl, &infoList);
            }
        }
    } else if (condition == errc) {
        result = DDSTouchStone_errorReportDataReader_read_w_condition (
                     err, &erl, &infoList, 1, errc);
        if (result == DDS_RETCODE_OK) {
            length = erl._length;
            if (length != 0) {
                for (i=0; i<length; i++) {
                    process_errorReport(
                        &erl._buffer[i],
                        &infoList._buffer[i].source_timestamp,
                        &infoList._buffer[i].reception_timestamp);
                }
                result = DDSTouchStone_errorReportDataReader_return_loan (
                             err, &erl, &infoList);
            }
        }
    } else {
        ERROR("Waitset Returned an unknown condition");
    }
    return TRUE;
}

static void
printUsage(
    char *name)
{
    printf("Usage: %s [-d] <file_name_prefix>\n",name);
}

static void
readCommandLineParams(
    int argc,
    char *argv[])
{
    int currArg = 0;
    char* application_name = argv[currArg];
    currArg++;

    if (argc <= 1)
    {
        printUsage(application_name);
        printf("filename prefix missing, exiting now.\n");
        exit(-1);
    }


    if (strcmp(argv[currArg], "-d") == 0)
    {
      output_level = 2;
      printf ("Display to screen is ON\n");
      currArg++;
    }

    output_prefix = argv[currArg];
    printf("Using Filename prefix: %s\n", output_prefix);
    printf("Starting %s\n", application_name);
}

int
main (
    int argc,
    char *argv[])
{

    DDS_WaitSet ExceleratorWaitset;
    DDS_Duration_t ExceleratorTimeout = DDS_DURATION_INFINITE;
    DDS_ReturnCode_t result;
    DDS_ConditionSeq *conditionList;
    DDS_StringSeq ExceleratorId;
    DDS_SubscriberQos *sQos;
    DDS_TopicQos *tQos;
    DDS_DataReaderQos *rQos;
    int length, i, proceed;
    char input;

    ExceleratorId._maximum   = 1;
    ExceleratorId._length    = 1;
    ExceleratorId._buffer    = DDS_StringSeq_allocbuf(1);
    ExceleratorId._buffer[0] = DDS_string_dup(argv[1]);

    readCommandLineParams(argc, argv);
    create_index ();
    create_css_file ();

    /*
     * Create WaitSet
     */
    ExceleratorWaitset = DDS_WaitSet__alloc ();

    /*
     * Create participant
     */
    ExceleratorFactory = DDS_DomainParticipantFactory_get_instance ();

    if (ExceleratorFactory == DDS_HANDLE_NIL) {
        printf ("Excelerator %s: ERROR - missing factoy instance\n",
                argv[0]);
        exit (1);
    }

    ExceleratorParticipant = DDS_DomainParticipantFactory_create_participant (
                             ExceleratorFactory,
                             ExceleratorDomain,
                             DDS_PARTICIPANT_QOS_DEFAULT,
                             NULL,
                             DDS_ANY_STATUS);

    if (ExceleratorParticipant == DDS_HANDLE_NIL) {
        printf ("Excelerator %s: ERROR - OpenSplice not running\n",
                argv[0]);
        exit (1);
    }

    sQos = DDS_SubscriberQos__alloc();

    DDS_DomainParticipant_get_default_subscriber_qos (ExceleratorParticipant,
                                                      sQos);

    sQos->partition.name._length = 1;
    sQos->partition.name._maximum = 1;
    sQos->partition.name._buffer = DDS_StringSeq_allocbuf (1);
    sQos->partition.name._buffer[0] = DDS_string_dup(DDSTouchStoneReportPartition);

    ExceleratorSubscriber = DDS_DomainParticipant_create_subscriber (
                             ExceleratorParticipant,
                             sQos,
                             NULL,
                             DDS_ANY_STATUS);

    DDS_free(sQos);

    /*
     * TransmitterReport
     */

    /*  Create Topic */
    trs = DDSTouchStone_transmitterReportTypeSupport__alloc ();

    DDSTouchStone_transmitterReportTypeSupport_register_type (
              trs,
              ExceleratorParticipant,
              "DDSTouchStone::transmitterReport");

    trt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transmitterReportTopic",
              "DDSTouchStone::transmitterReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    trr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              trt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    trrc = DDS_DataReader_create_readcondition (
              trr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, trrc);

    /*
     * ReceiverReport
     */

    /*  Create Topic */
    rrs = DDSTouchStone_receiverReportTypeSupport__alloc ();

    DDSTouchStone_receiverReportTypeSupport_register_type (
              rrs,
              ExceleratorParticipant,
              "DDSTouchStone::receiverReport");

    rrt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "receiverReportTopic",
              "DDSTouchStone::receiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    rrr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              rrt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    rrrc = DDS_DataReader_create_readcondition (
              rrr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, rrrc);

    /*
     * transceiverReport
     */

    /*  Create Topic */
    srs = DDSTouchStone_transceiverReportTypeSupport__alloc ();

    DDSTouchStone_transceiverReportTypeSupport_register_type (
              srs, ExceleratorParticipant,
              "DDSTouchStone::transceiverReport");

    srt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transceiverReportTopic",
              "DDSTouchStone::transceiverReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    srr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              srt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    srrc = DDS_DataReader_create_readcondition (
              srr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, srrc);

    /*
     * transponderReport
     */

    /*  Create Topic */
    xrs = DDSTouchStone_transponderReportTypeSupport__alloc ();

    DDSTouchStone_transponderReportTypeSupport_register_type (
              xrs, ExceleratorParticipant,
              "DDSTouchStone::transponderReport");

    xrt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "transponderReportTopic",
              "DDSTouchStone::transponderReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    xrr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              xrt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    xrrc = DDS_DataReader_create_readcondition (
              xrr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, xrrc);

    /*  Create Topic */
    drs = DDSTouchStone_discoveryReportTypeSupport__alloc ();

    DDSTouchStone_discoveryReportTypeSupport_register_type (
              drs,
              ExceleratorParticipant,
              "DDSTouchStone::discoveryReport");

    drt = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "discoveryReportTopic",
              "DDSTouchStone::discoveryReport",
              DDS_TOPIC_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Create datareader */
    drr = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              drt,
              DDS_DATAREADER_QOS_DEFAULT,
              NULL,
              DDS_ANY_STATUS);

    /* Add datareader readcondition to waitset */

    drrc = DDS_DataReader_create_readcondition (
              drr,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, drrc);

    /*  Create Topic */
    ers = DDSTouchStone_errorReportTypeSupport__alloc ();

    DDSTouchStone_errorReportTypeSupport_register_type (
              ers, ExceleratorParticipant,
              "DDSTouchStone::errorReport");

    tQos = DDS_TopicQos__alloc();

    DDS_DomainParticipant_get_default_topic_qos(ExceleratorParticipant, tQos);
    tQos->reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
    tQos->durability.kind = DDS_TRANSIENT_DURABILITY_QOS;

    ert = DDS_DomainParticipant_create_topic (
              ExceleratorParticipant,
              "errorReportTopic",
              "DDSTouchStone::errorReport",
              tQos,
              NULL,
              DDS_ANY_STATUS);

    rQos = DDS_DataReaderQos__alloc();

    DDS_Subscriber_copy_from_topic_qos(ExceleratorSubscriber, rQos, tQos);

    /* Create datareader */
    err = DDS_Subscriber_create_datareader (
              ExceleratorSubscriber,
              ert,
              rQos,
              NULL,
              DDS_ANY_STATUS);

    DDS_free(rQos);
    DDS_free(tQos);

    /* Add datareader readcondition to waitset */

    errc = DDS_DataReader_create_readcondition (
              err,
              DDS_NOT_READ_SAMPLE_STATE,
              DDS_ANY_VIEW_STATE,
              DDS_ANY_INSTANCE_STATE);

    result = DDS_WaitSet_attach_condition (ExceleratorWaitset, errc);

    /*
     * MainLoop processing
     */
    proceed = TRUE;
    while (proceed) {
        conditionList = DDS_ConditionSeq__alloc();
        result = DDS_WaitSet_wait (ExceleratorWaitset,
                                   conditionList,
                                   &ExceleratorTimeout);
        length = conditionList->_length;
        for (i = 0; (i < length) && proceed; i++) {
            proceed = process_command(conditionList->_buffer[i]);
        }

    }
    return 0;
}

