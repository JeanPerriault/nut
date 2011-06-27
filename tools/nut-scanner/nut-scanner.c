/* nut-scanner.c: a tool to detect NUT supported devices
 * 
 *  Copyright (C) 2011 - Arnaud Quette <arnaud.quette@free.fr>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* TODO list:
 * - network iterator (IPv4 and v6) for connect style scan
 * - handle XML/HTTP and SNMP answers (need thread?)
 * - Avahi support for NUT instances discovery
 * (...)
 * https://alioth.debian.org/pm/task.php?func=detailtask&project_task_id=477&group_id=30602&group_project_id=42
 */

#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#include "nut-scan.h"

#define DEFAULT_TIMEOUT 1

const char optstring[] = "?ht:s:e:c:l:u:A:X:a:x:p:";
const struct option longopts[] =
	{{ "timeout",required_argument,NULL,'t' },
	{ "start_ip",required_argument,NULL,'s' },
	{ "end_ip",required_argument,NULL,'e' },
	{ "community",required_argument,NULL,'c' },
	{ "secLevel",required_argument,NULL,'l' },
	{ "secName",required_argument,NULL,'u' },
	{ "authPassword",required_argument,NULL,'A' },
	{ "privPassword",required_argument,NULL,'X' },
	{ "authProtocol",required_argument,NULL,'a' },
	{ "privProtocol",required_argument,NULL,'x' },
	{ "port",required_argument,NULL,'p' },
	{ "help",no_argument,NULL,'h' },
	{NULL,0,NULL,0}};


int main(int argc, char *argv[])
{
	device_t * dev;
	snmp_security_t sec;
	long timeout = DEFAULT_TIMEOUT*1000*1000; /* in usec */
	int opt_ret;
	char *	start_ip = NULL;
	char *	end_ip = NULL;
	char * port = NULL;

	memset(&sec,0,sizeof(sec));

	while((opt_ret = getopt_long(argc, argv,optstring,longopts,NULL))!=-1) {

		switch(opt_ret) {
			case 't':
				timeout = atol(optarg)*1000*1000; /*in usec*/
				if( timeout == 0 ) {
					fprintf(stderr,"Illegal timeout value, using default %ds\n", DEFAULT_TIMEOUT);
					timeout = DEFAULT_TIMEOUT*1000*1000;
				}
				break;
			case 's':
				start_ip = strdup(optarg);
				end_ip = start_ip;
				break;
			case 'e':
				end_ip = strdup(optarg);
				break;
			case 'c':
				sec.community = strdup(optarg);
				break;
			case 'l':
				sec.secLevel = strdup(optarg);
				break;
			case 'u':
				sec.secName = strdup(optarg);
				break;
			case 'A':
				sec.authPassword = strdup(optarg);
				break;
			case 'X':
				sec.privPassword = strdup(optarg);
				break;
			case 'a':
				sec.authProtocol = strdup(optarg);
				break;
			case 'x':
				sec.privProtocol = strdup(optarg);
				break;
			case 'p':
				port = strdup(optarg);
				break;
			case 'h':
			case '?':
			default:
				puts("nut-scanner : detecting available UPS.\n");
				puts("OPTIONS:");
				printf("  -t, --timeout <timeout in seconds>: network operation timeout (default %d).\n",DEFAULT_TIMEOUT);
				printf("  -s, --start_ip <IP address>: First IP address to scan.\n");
				printf("  -e, --end_ip <IP address>: Last IP address to scan.\n");

				printf("\nSNMP v1 specific options:\n");
				printf("  -c, --community <community name>: Set SNMP v1 community name (default = public)\n");

				printf("\nSNMP v3 specific options:\n");
				printf("  -l, --secLevel <security level>: Set the securityLevel used for SNMPv3 messages (allowed: noAuthNoPriv,authNoPriv,authPriv)\n");
				printf("  -u, --secName <security name>: Set the securityName used for authenticated SNMPv3 messages (mandatory if you set secLevel. No default)\n");
				printf("  -a, --authProtocol <authentication protocol>: Set the authentication protocol (MD5 or SHA) used for authenticated SNMPv3 messages (default=MD5)\n");
				printf("  -A, --authPassword <authentication pass phrase>: Set the authentication pass phrase used for authenticated SNMPv3 messages (mandatory if you set secLevel to authNoPriv or authPriv)\n");
				printf("  -x, --privProtocol <privacy protocol>: Set the privacy protocol (DES or AES) used for encrypted SNMPv3 messages (default=DES)\n");
				printf("  -X, --privPassword <privacy pass phrase>: Set the privacy pass phrase used for encrypted SNMPv3 messages (mandatory if you set secLevel to authPriv)\n");

				printf("\nNUT device specific options:\n");
				printf("  -p, --port <port number>: Port number of remote NUT devices\n");
				return 0;
		}

	}


#ifdef HAVE_USB_H
	printf("Scanning USB bus:\n");
	dev = scan_usb();
	display_ups_conf(dev);
	free_device(dev);
#endif /* HAVE_USB_H */

#ifdef HAVE_NET_SNMP_NET_SNMP_CONFIG_H
	printf("Scanning SNMP bus:\n");
	dev = scan_snmp(start_ip,end_ip,timeout,&sec);
	display_ups_conf(dev);
        free_device(dev);

#endif /* HAVE_NET_SNMP_NET_SNMP_CONFIG_H */

	printf("Scanning XML/HTTP bus:\n");
	dev = scan_xml_http(timeout);
	display_ups_conf(dev);
	free_device(dev);

	printf("Scanning NUT bus (old connect method):\n");
	dev = scan_nut(start_ip,end_ip,port);
	display_ups_conf(dev);
	free_device(dev);

/*TODO*/
	printf("Scanning NUT bus (via avahi):\n");
	scan_avahi();

/*TODO*/
	printf("Scanning IPMI bus:\n");
	scan_ipmi();

	return EXIT_SUCCESS;
}
