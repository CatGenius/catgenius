/* Run the real parser with a bounded command callback and captured output. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char output[512];
static unsigned int calls;
static int expected_argc;
static const char *serial_input = "";

static int command_printf(const char *format, ...)
{
	va_list args;
	int result;
	size_t used = strlen(output);
	va_start(args, format);
	result = vsnprintf(output + used, sizeof(output) - used, format, args);
	va_end(args);
	assert(result >= 0 && (size_t)result < sizeof(output) - used);
	return result;
}
#define printf command_printf
#include "../common/cmdline.c"
#undef printf

static int record(int argc, char *argv[])
{
	int i;
	calls++;
	assert(argc == expected_argc && argc <= ARGS_MAX);
	assert(!strcmp(argv[0], "record"));
	for (i = 1; i < argc; i++)
		assert(!strcmp(argv[i], "a"));
	return ERR_OK;
}
const struct command commands[] = {{"record", record}, {"", 0}};
unsigned char readch(char *rxd)
{
	if (!*serial_input)
		return 0;
	*rxd = *serial_input++;
	return 1;
}
void putch(char value)
{
	size_t used = strlen(output);
	assert(used + 1 < sizeof(output));
	output[used] = value;
	output[used + 1] = 0;
}

static void parse(const char *input)
{
	char line[LINEBUFFER_MAX];
	assert(strlen(input) < sizeof(line));
	strcpy(line, input);
	output[0] = 0;
	proc_line(line);
}

static void receive(const char *input, const char *expected_output,
	unsigned int expected_calls)
{
	unsigned int before = calls;
	output[0] = 0;
	serial_input = input;
	cmdline_work();
	assert(!strcmp(output, expected_output));
	assert(calls == before + expected_calls);
}

static void test_editing(char erase)
{
	char edit[] = {erase, 0};
	char full_line[LINEBUFFER_MAX];

	localecho = 1;
	output[0] = 0;
	cmdline_init();
	assert(!strcmp(output, "# "));
	/* An empty line must not erase the prompt or underflow the cursor. */
	receive(edit, "", 0);
	/* Edits persist across worker calls and replace the erased character. */
	receive("recorx", "recorx", 0);
	receive(edit, "\b \b", 0);
	receive("d\r", "d\n# ", 1);
	receive("record a", "record a", 0);
	receive(edit, "\b \b", 0);
	receive(edit, "\b \b", 0);
	receive("\r", "\n# ", 1);
	/* Deleting the complete line must not dispatch its previous contents. */
	receive("x", "x", 0);
	receive(edit, "\b \b", 0);
	receive(edit, "", 0);
	receive("\r", "\n# ", 0);
	/* A full buffer rejects another character; an edit makes room again. */
	memset(full_line, ' ', sizeof(full_line) - 1);
	memcpy(full_line, "record", 6);
	full_line[sizeof(full_line) - 1] = 0;
	receive(full_line, full_line, 0);
	receive("x", "\a", 0);
	receive(edit, "\b \b", 0);
	receive(" \r", " \n# ", 1);
	/* Echo-off still edits and dispatches, without erasure, bell or prompt. */
	localecho = 0;
	receive(edit, "", 0);
	receive("recorx", "", 0);
	receive(edit, "", 0);
	receive("d\r", "", 1);
	receive(full_line, "", 0);
	receive("x", "", 0);
	receive(edit, "", 0);
	receive(" \r", "", 1);
	receive("\005", "\006", 0);
	localecho = 1;
}

int main(void)
{
	expected_argc = 1;
	parse("record");
	assert(calls == 1 && !output[0]);
	expected_argc = ARGS_MAX;
	parse("  record a a a  ");
	assert(calls == 2 && !output[0]);
	parse("missing");
	assert(calls == 2 && strstr(output, "Unknown command"));
	parse("record a a a a");
	assert(calls == 2 && strstr(output, "Syntax error"));
	parse("");
	assert(calls == 2 && !output[0]);
	parse("  \t  ");
	assert(calls == 2 && !output[0]);
	expected_argc = 1;
	parse("record");
	assert(calls == 3 && !output[0]);
	/* Exercise complete lines through the actual character worker too. */
	receive("record\r\r   \rrecord a a a a\rrecord\r",
		"record\n# " "\n# " "   \n# "
		"record a a a a\nSyntax error\n# " "record\n# ", 2);
	test_editing('\b');
	test_editing(0x7f);
	receive("record a\b\x7f\r", "record a\b \b\b \b\n# ", 1);
	receive("record\r\n", "record\n# ", 1);
	puts("Host command-line checks passed.");
	return 0;
}
