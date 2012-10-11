#include <stdio.h>
#define T2H_IMPORT
#include "t2h.h"

int main(char ** argv, int argc) 
{
	t2h_init(NULL);
	t2h_close(-1);
	t2h_add_torrent(-1, NULL);
	t2h_add_torrent_url(-1, NULL);
	t2h_get_torrent_files(-1, -1);
	t2h_start_download(-1, -1, -1);	
	t2h_paused_download(-1, -1, -1);
	t2h_resume_download(-1, -1, -1);
	t2h_delete_torrent(-1, -1);
	t2h_stop_download(-1, -1);

	return 0;
}

