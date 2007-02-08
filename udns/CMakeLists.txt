PROJECT (udns) 
ADD_LIBRARY (	udns STATIC udns_dn.c udns_dntosp.c udns_parse.c 
		udns_resolver.c udns_init.c udns_misc.c udns_XtoX.c 
                udns_rr_a.c udns_rr_ptr.c udns_rr_mx.c 
                udns_rr_txt.c udns_bl.c udns_rr_srv.c 
                udns_rr_naptr.c udns_codes.c )
