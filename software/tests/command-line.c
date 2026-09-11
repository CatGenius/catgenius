/* Run the real parser with a bounded command callback and captured output. */
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static char output[512];
static unsigned int calls;
static int expected_argc;

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
unsigned char readch(char *rxd) { (void)rxd; return 0; }
void putch(char value) { (void)value; }

static void parse(const char *input)
{
	char line[LINEBUFFER_MAX];
	assert(strlen(input) < sizeof(line));
	strcpy(line, input);
	output[0] = 0;
	proc_line(line);
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
	puts("Host command-line checks passed.");
	return 0;
}
