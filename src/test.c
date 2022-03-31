#include "system/system.h"
#include "system/endiannes.h"

#include "ebneuro/ebneuro.h"

int main(int argc, char *argv[])
{
	s_set_verbosity(INFO);

	s_dprintf(ALWAYS, "hello\n");
	s_dprintf(INFO, "int %d\n", 1);
	s_set_verbosity(CRITICAL);
	s_dprintf(INFO, "char %s\n", "ass");

	int fd;

	s_connect(&fd, "127.0.0.1", 8888);
	//s_send(fd, "hello", 5, 0);

	eb_send(fd, 12, "data", 4);
	eb_send_id(fd, 10);

	s_close(fd);
	
	return 0;
}
