//
//  dacpclient.h
//  AirFloat
//
//  Created by Kristian Trenskow on 3/7/13.
//
//

#ifndef _dacpclient_h
#define _dacpclient_h

typedef struct dacp_client_t *dacp_client_p;

dacp_client_p dacp_client_create(const char* identifier, const char* active_remote);
void dacp_client_destroy(dacp_client_p dc);

#endif
