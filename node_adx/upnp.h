#ifndef NODE_API
#define NODE_API C_IMPORT
#endif

struct pnpgw
{
	struct string	desc_url;
	struct string	control_path;
	struct string	control_url;
	struct string	desc;
	struct string	ex_ip;
};

NODE_API int		C_API_FUNC forwardPort			(struct string *port_str);
NODE_API int		C_API_FUNC broadcastDiscovery	();
NODE_API void		C_API_FUNC init_upnp			();
NODE_API int		C_API_FUNC get_gw_ip			(struct string *gwIP);
