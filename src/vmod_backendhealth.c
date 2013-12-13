#include <stdio.h>
#include <stdlib.h>

#include "vrt.h"
#include "bin/varnishd/cache.h"
#include "bin/varnishd/cache_backend.h"
#include "vcl.h"

#include "vcc_if.h"

int
init_function(struct vmod_priv *priv, const struct VCL_conf *conf)
{
	return (0);
}

static char *
wsstrncat(char *dest, const char *src, unsigned max_sz) {
	if (strlen(dest) + strlen(src) >= max_sz) {
		return (NULL);
	}

	return strcat(dest, src);
}

/* This lets us cat to a ws-allocated string and just abandon if we run
   out of space. */
#define STRCAT(dst, src, max) \
	dst = wsstrncat(dst, src, max);			\
	if (!dst) {					\
		WS_Release(sp->wrk->ws, 0);		\
		return "";				\
	}						

const char *
vmod_json(struct sess *sp)
{
	char *p;
	unsigned max_sz;
	unsigned sz;
	int i;

	max_sz = WS_Reserve(sp->wrk->ws, 0);
	p = sp->wrk->ws->f;
	*p = 0;
	STRCAT(p, "{\n", max_sz);
		
	for (i = 1; i < sp->vcl->ndirector; ++i) {
		CHECK_OBJ_NOTNULL(sp->vcl->director[i], DIRECTOR_MAGIC);
		if (strcmp("simple", sp->vcl->director[i]->name) == 0) {
			char buf[1024];
			int j, healthy;

			healthy = VDI_Healthy(sp->vcl->director[i], sp);
			j = snprintf(buf, sizeof buf, "    \"%s\": \"%s\"%s\n",
			    sp->vcl->director[i]->vcl_name, healthy ? "healthy" : "sick",
			    (i == sp->vcl->ndirector - 1) ? "" : "," );
			assert(j >= 0);

			STRCAT(p, buf, max_sz);
		}
	}

	STRCAT(p, "}\n", max_sz);
	
	WS_Release(sp->wrk->ws, strlen(p));
	return (p);
}
