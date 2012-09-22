#include <stdio.h>
#define T2H_IMPORT
#include "t2h.h"

int main(char ** argv, int argc) 
{
	t2h_init(NULL);
	t2h_close(NULL);
	t2h_add_torrent(NULL, NULL);
	t2h_add_torrent_url(NULL, NULL);
	t2h_get_torrent_files(NULL, -1);
	t2h_start_download(NULL, -1, -1);	
	t2h_paused_download(NULL, -1, -1);
	t2h_resume_download(NULL, -1, -1);
	t2h_delete_torrent(NULL, -1);
	t2h_stop_download(NULL, -1);
	return 0;
}

