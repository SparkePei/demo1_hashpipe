/*
 * demo1_output_thread.c
 */

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "hashpipe.h"
#include "demo1_databuf.h"

static void *run(hashpipe_thread_args_t * args)
{
    // Local aliases to shorten access to args fields
    // Our input buffer happens to be a demo1_ouput_databuf
    demo1_output_databuf_t *db = (demo1_output_databuf_t *)args->ibuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;
    int c,rv;
    int block_idx = 0;
    uint64_t mcnt=0;
    FILE * demo1_file;
    demo1_file=fopen("./demo1_file.txt","w");
    
    /* Main loop */
    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "OUTBLKIN", block_idx);
	hputi8(st.buf, "OUTMCNT",mcnt);
        hputs(st.buf, status_key, "waiting");
        hashpipe_status_unlock_safe(&st);
	sleep(1);
       // get new data
       while ((rv=demo1_output_databuf_wait_filled(db, block_idx))
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for filled databuf");
                pthread_exit(NULL);
                break;
            }
        }

        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing");
        hashpipe_status_unlock_safe(&st);

        // TODO check mcnt
	c=db->block[block_idx].sum;
        fwrite(&c,sizeof(c),1,demo1_file);
	sleep(1);
        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "OUTSUM", c);
        hashpipe_status_unlock_safe(&st);
	
	demo1_output_databuf_set_free(db,block_idx);
	block_idx = (block_idx + 1) % db->header.n_block;
	mcnt++;
    }
    fclose(demo1_file);
    return NULL;
}

static hashpipe_thread_desc_t demo1_output_thread = {
    name: "demo1_output_thread",
    skey: "OUTSTAT",
    init: NULL, 
    run:  run,
    ibuf_desc: {demo1_output_databuf_create},
    obuf_desc: {NULL}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&demo1_output_thread);
}

