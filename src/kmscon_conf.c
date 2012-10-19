/*
 * kmscon - Configuration Parser
 *
 * Copyright (c) 2012 David Herrmann <dh.herrmann@googlemail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <paths.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include "conf.h"
#include "kmscon_conf.h"
#include "log.h"
#include "shl_misc.h"

struct kmscon_conf_t kmscon_conf;

static void print_help()
{
	/*
	 * Usage/Help information
	 * This should be scaled to a maximum of 80 characters per line:
	 *
	 * 80 char line:
	 *       |   10   |    20   |    30   |    40   |    50   |    60   |    70   |    80   |
	 *      "12345678901234567890123456789012345678901234567890123456789012345678901234567890\n"
	 * 80 char line starting with tab:
	 *       |10|    20   |    30   |    40   |    50   |    60   |    70   |    80   |
	 *      "\t901234567890123456789012345678901234567890123456789012345678901234567890\n"
	 */
	fprintf(stderr,
		"Usage:\n"
		"\t%1$s [options]\n"
		"\t%1$s -h [options]\n"
		"\t%1$s -l [options] -- /bin/sh [sh-arguments]\n"
		"\n"
		"You can prefix boolean options with \"no-\" to negate them. If an argument is\n"
		"given multiple times, only the last argument matters if not otherwise stated.\n"
		"\n"
		"General Options:\n"
		"\t-h, --help                  [off]   Print this help and exit\n"
		"\t-v, --verbose               [off]   Print verbose messages\n"
		"\t    --debug                 [off]   Enable debug mode\n"
		"\t    --silent                [off]   Suppress notices and warnings\n"
		"\n"
		"Seat Options:\n"
		"\t    --vt <vt-number>        [auto]  Select which VT to run on on seat0\n"
		"\t-s, --switchvt              [off]   Automatically switch to VT\n"
		"\t    --seats <list,of,seats> [seat0] Select seats or pass 'all' to make\n"
		"\t                                    kmscon run on all seats\n"
		"\n"
		"Session Options:\n"
		"\t    --session-max <max>         [50] Maximum number of sessions\n"
		"\n"
		"Terminal Options:\n"
		"\t-l, --login                 [/bin/sh]\n"
		"\t                              Start the given login process instead\n"
		"\t                              of the default process; all arguments\n"
		"\t                              following '--' will be be parsed as\n"
		"\t                              argv to this process. No more options\n"
		"\t                              after '--' will be parsed so use it at\n"
		"\t                              the end of the argument string\n"
		"\t-t, --term <TERM>           [xterm-256color]\n"
		"\t                              Value of the TERM environment variable\n"
		"\t                              for the child process\n"
		"\t    --palette <name>        [default]\n"
		"\t                              Select the used color palette\n"
		"\t    --sb-size <num>         [1000]\n"
		"\t                              Size of the scrollback-buffer in lines\n"
		"\n"
		"Input Options:\n"
		"\t    --xkb-layout <layout>      [us] Set XkbLayout for input devices\n"
		"\t    --xkb-variant <variant>    [-]  Set XkbVariant for input devices\n"
		"\t    --xkb-options <options>    [-]  Set XkbOptions for input devices\n"
		"\t    --xkb-repeat-delay <msecs> [250]\n"
		"\t                                 Initial delay for key-repeat in ms\n"
		"\t    --xkb-repeat-rate <msecs>  [50]\n"
		"\t                                 Delay between two key-repeats in ms\n"
		"\n"
		"Grabs / Keyboard-Shortcuts:\n"
		"\t    --grab-scroll-up <grab>     [<Shift>Up]\n"
		"\t                                  Shortcut to scroll up\n"
		"\t    --grab-scroll-down <grab>   [<Shift>Down]\n"
		"\t                                  Shortcut to scroll down\n"
		"\t    --grab-page-up <grab>       [<Shift>Prior]\n"
		"\t                                  Shortcut to scroll page up\n"
		"\t    --grab-page-down <grab>     [<Shift>Next]\n"
		"\t                                  Shortcut to scroll page down\n"
		"\t    --grab-session-next <grab>  [<Ctrl><Alt>Right]\n"
		"\t                                  Switch to next session\n"
		"\t    --grab-session-prev <grab>  [<Ctrl><Alt>Left]\n"
		"\t                                  Switch to previous session\n"
		"\t    --grab-session-close <grab> [<Ctrl><Alt>W]\n"
		"\t                                  Close current session\n"
		"\t    --grab-terminal-new <grab>  [<Ctrl><Alt>Return]\n"
		"\t                                  Create new terminal session\n"
		"\n"
		"Video Options:\n"
		"\t    --fbdev                 [off]   Use fbdev instead of DRM\n"
		"\t    --dumb                  [off]   Use dumb DRM instead of hardware-\n"
		"\t                                    accelerated DRM devices\n"
		"\t    --fps                   [50]    Limit frame-rate\n"
		"\t    --render-engine <eng>   [-]     Console renderer\n"
		"\t    --render-timing         [off]   Print renderer timing information\n"
		"\n"
		"Font Options:\n"
		"\t    --font-engine <engine>  [pango]\n"
		"\t                              Font engine\n"
		"\t    --font-size <points>    [15]\n"
		"\t                              Font size in points\n"
		"\t    --font-name <name>      [monospace]\n"
		"\t                              Font name\n"
		"\t    --font-dpi <dpi>        [96]\n"
		"\t                              Force DPI value for all fonts\n",
		"kmscon");
	/*
	 * 80 char line:
	 *       |   10   |    20   |    30   |    40   |    50   |    60   |    70   |    80   |
	 *      "12345678901234567890123456789012345678901234567890123456789012345678901234567890\n"
	 * 80 char line starting with tab:
	 *       |10|    20   |    30   |    40   |    50   |    60   |    70   |    80   |
	 *      "\t901234567890123456789012345678901234567890123456789012345678901234567890\n"
	 */
}

static void conf_default_vt(struct conf_option *opt)
{
	opt->type->free(opt);
	*(void**)opt->mem = opt->def;
}

static void conf_free_vt(struct conf_option *opt)
{
	if (*(void**)opt->mem) {
		if (*(void**)opt->mem != opt->def)
			free(*(void**)opt->mem);
		*(void**)opt->mem = NULL;
	}
}

static int conf_parse_vt(struct conf_option *opt, bool on, const char *arg)
{
	static const char prefix[] = "/dev/";
	unsigned int val;
	char *str;
	int ret;

	if (!shl_strtou(arg, &val)) {
		ret = asprintf(&str, "%stty%u", prefix, val);
		if (ret == -1)
			return -ENOMEM;
	} else if (*arg && *arg != '.' && *arg != '/') {
		str = malloc(sizeof(prefix) + strlen(arg));
		if (!str)
			return -ENOMEM;

		strcpy(str, prefix);
		strcat(str, arg);
	} else {
		str = strdup(arg);
		if (!str)
			return -ENOMEM;
	}

	opt->type->free(opt);
	*(void**)opt->mem = str;
	return 0;
}

static int conf_copy_vt(struct conf_option *opt,
			const struct conf_option *src)
{
	char *val;

	if (!*(void**)src->mem) {
		val = NULL;
	} else {
		val = strdup(*(void**)src->mem);
		if (!val)
			return -ENOMEM;
	}

	opt->type->free(opt);
	*(void**)opt->mem = val;
	return 0;
}

static const struct conf_type conf_vt = {
	.flags = CONF_HAS_ARG,
	.set_default = conf_default_vt,
	.free = conf_free_vt,
	.parse = conf_parse_vt,
	.copy = conf_copy_vt,
};

#define KMSCON_CONF_FROM_FIELD(_mem, _name) \
	shl_offsetof((_mem), struct kmscon_conf_t, _name)

static int aftercheck_debug(struct conf_option *opt, int argc, char **argv,
			    int idx)
{
	struct kmscon_conf_t *conf = KMSCON_CONF_FROM_FIELD(opt->mem, debug);

	/* --debug implies --verbose */
	if (conf->debug)
		conf->verbose = 1;

	return 0;
}

static int aftercheck_help(struct conf_option *opt, int argc, char **argv,
			   int idx)
{
	struct kmscon_conf_t *conf = KMSCON_CONF_FROM_FIELD(opt->mem, help);

	/* exit after printing --help information */
	if (conf->help) {
		print_help();
		conf->exit = true;
	}

	return 0;
}

static char *def_argv[] = { NULL, "-i", NULL };

static int aftercheck_login(struct conf_option *opt, int argc, char **argv,
			    int idx)
{
	int ret;
	struct kmscon_conf_t *conf = KMSCON_CONF_FROM_FIELD(opt->mem, login);

	/* parse "--login [...] -- args" arguments */
	if (conf->login) {
		if (idx >= argc) {
			fprintf(stderr, "Arguments for --login missing\n");
			return -EFAULT;
		}

		conf->argv = &argv[idx];
		ret = argc - idx;
	} else {
		def_argv[0] = getenv("SHELL") ? : _PATH_BSHELL;
		conf->argv = def_argv;
		ret = 0;
	}

	return ret;
}

static int aftercheck_seats(struct conf_option *opt, int argc, char **argv,
			    int idx)
{
	struct kmscon_conf_t *conf = KMSCON_CONF_FROM_FIELD(opt->mem, seats);

	if (conf->seats[0] &&
	    !conf->seats[1] &&
	    !strcmp(conf->seats[0], "all"))
		conf->all_seats = true;

	return 0;
}

static char *def_seats[] = { "seat0", NULL };

static struct conf_grab def_grab_scroll_up =
		CONF_SINGLE_GRAB(SHL_SHIFT_MASK, XKB_KEY_Up);

static struct conf_grab def_grab_scroll_down =
		CONF_SINGLE_GRAB(SHL_SHIFT_MASK, XKB_KEY_Down);

static struct conf_grab def_grab_page_up =
		CONF_SINGLE_GRAB(SHL_SHIFT_MASK, XKB_KEY_Prior);

static struct conf_grab def_grab_page_down =
		CONF_SINGLE_GRAB(SHL_SHIFT_MASK, XKB_KEY_Next);

static struct conf_grab def_grab_session_next =
		CONF_SINGLE_GRAB(SHL_CONTROL_MASK | SHL_ALT_MASK, XKB_KEY_Right);

static struct conf_grab def_grab_session_prev =
		CONF_SINGLE_GRAB(SHL_CONTROL_MASK | SHL_ALT_MASK, XKB_KEY_Left);

static struct conf_grab def_grab_session_close =
		CONF_SINGLE_GRAB(SHL_CONTROL_MASK | SHL_ALT_MASK, XKB_KEY_w);

static struct conf_grab def_grab_terminal_new =
		CONF_SINGLE_GRAB(SHL_CONTROL_MASK | SHL_ALT_MASK, XKB_KEY_Return);

int kmscon_conf_new(struct conf_ctx **out, struct kmscon_conf_t *conf)
{
	struct conf_ctx *ctx;
	int ret;

	if (!out || !conf)
		return -EINVAL;

	struct conf_option options[] = {
		/* Global Options */
		CONF_OPTION_BOOL('h', "help", aftercheck_help, &conf->help, false),
		CONF_OPTION_BOOL('v', "verbose", NULL, &conf->verbose, false),
		CONF_OPTION_BOOL(0, "debug", aftercheck_debug, &conf->debug, false),
		CONF_OPTION_BOOL(0, "silent", NULL, &conf->silent, false),

		/* Seat Options */
		CONF_OPTION(0, 0, "vt", &conf_vt, NULL, &conf->vt, NULL),
		CONF_OPTION_BOOL('s', "switchvt", NULL, &conf->switchvt, false),
		CONF_OPTION_STRING_LIST(0, "seats", aftercheck_seats, &conf->seats, def_seats),

		/* Session Options */
		CONF_OPTION_UINT(0, "session-max", NULL, &conf->session_max, 50),

		/* Terminal Options */
		CONF_OPTION_BOOL('l', "login", aftercheck_login, &conf->login, false),
		CONF_OPTION_STRING('t', "term", NULL, &conf->term, "xterm-256color"),
		CONF_OPTION_STRING(0, "palette", NULL, &conf->palette, NULL),
		CONF_OPTION_UINT(0, "sb-size", NULL, &conf->sb_size, 1000),

		/* Input Options */
		CONF_OPTION_STRING(0, "xkb-layout", NULL, &conf->xkb_layout, "us"),
		CONF_OPTION_STRING(0, "xkb-variant", NULL, &conf->xkb_variant, ""),
		CONF_OPTION_STRING(0, "xkb-options", NULL, &conf->xkb_options, ""),
		CONF_OPTION_UINT(0, "xkb-repeat-delay", NULL, &conf->xkb_repeat_delay, 250),
		CONF_OPTION_UINT(0, "xkb-repeat-rate", NULL, &conf->xkb_repeat_rate, 50),

		/* Grabs / Keyboard-Shortcuts */
		CONF_OPTION_GRAB(0, "grab-scroll-up", NULL, &conf->grab_scroll_up, &def_grab_scroll_up),
		CONF_OPTION_GRAB(0, "grab-scroll-down", NULL, &conf->grab_scroll_down, &def_grab_scroll_down),
		CONF_OPTION_GRAB(0, "grab-page-up", NULL, &conf->grab_page_up, &def_grab_page_up),
		CONF_OPTION_GRAB(0, "grab-page-down", NULL, &conf->grab_page_down, &def_grab_page_down),
		CONF_OPTION_GRAB(0, "grab-session-next", NULL, &conf->grab_session_next, &def_grab_session_next),
		CONF_OPTION_GRAB(0, "grab-session-prev", NULL, &conf->grab_session_prev, &def_grab_session_prev),
		CONF_OPTION_GRAB(0, "grab-session-close", NULL, &conf->grab_session_close, &def_grab_session_close),
		CONF_OPTION_GRAB(0, "grab-terminal-new", NULL, &conf->grab_terminal_new, &def_grab_terminal_new),

		/* Video Options */
		CONF_OPTION_BOOL(0, "fbdev", NULL, &conf->fbdev, false),
		CONF_OPTION_BOOL(0, "dumb", NULL, &conf->dumb, false),
		CONF_OPTION_UINT(0, "fps", NULL, &conf->fps, 50),
		CONF_OPTION_STRING(0, "render-engine", NULL, &conf->render_engine, NULL),
		CONF_OPTION_BOOL(0, "render-timing", NULL, &conf->render_timing, false),

		/* Font Options */
		CONF_OPTION_STRING(0, "font-engine", NULL, &conf->font_engine, "pango"),
		CONF_OPTION_UINT(0, "font-size", NULL, &conf->font_size, 12),
		CONF_OPTION_STRING(0, "font-name", NULL, &conf->font_name, "monospace"),
		CONF_OPTION_UINT(0, "font-dpi", NULL, &conf->font_ppi, 96),
	};

	ret = conf_ctx_new(&ctx, options, sizeof(options) / sizeof(*options),
			   conf);
	if (ret)
		return ret;

	*out = ctx;
	return 0;
}

void kmscon_conf_free(struct conf_ctx *ctx)
{
	conf_ctx_free(ctx);
}

int kmscon_conf_load_main(struct conf_ctx *ctx, int argc, char **argv)
{
	int ret;

	ret = conf_ctx_parse_argv(ctx, argc, argv);
	if (ret)
		return ret;

	if (kmscon_conf.exit)
		return 0;

	if (!kmscon_conf.debug && !kmscon_conf.verbose && kmscon_conf.silent)
		log_set_config(&LOG_CONFIG_WARNING(0, 0, 0, 0));
	else
		log_set_config(&LOG_CONFIG_INFO(kmscon_conf.debug,
						kmscon_conf.verbose));

	log_print_init("kmscon");

	ret = conf_ctx_parse_file(ctx, "/etc/kmscon/kmscon.conf");
	if (ret)
		return ret;

	/* TODO: Deprecated! Remove this! */
	if (!access("/etc/kmscon.conf", F_OK)) {
		log_error("/etc/kmscon.conf is deprecated, please use /etc/kmscon/kmscon.conf");
		ret = conf_ctx_parse_file(ctx, "/etc/kmscon.conf");
		if (ret)
			return ret;
	}

	return 0;
}

int kmscon_conf_load_seat(struct conf_ctx *ctx, const struct conf_ctx *main,
			  const char *seat)
{
	int ret;

	if (!ctx || !main || !seat)
		return -EINVAL;

	log_debug("parsing seat configuration for seat %s", seat);

	ret = conf_ctx_parse_ctx(ctx, main);
	if (ret)
		return ret;

	ret = conf_ctx_parse_file(ctx, "/etc/kmscon/%s.seat.conf", seat);
	if (ret)
		return ret;

	return 0;
}