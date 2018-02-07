/*
 * demo1_fake_net_thread.c
 *
 * Routine to write fake data into shared memory blocks.  This allows the
 * processing pipelines to be tested without the network portion of SERENDIP6.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include "hashpipe.h"
#include "demo1_databuf.h"

static void *run(hashpipe_thread_args_t * args)
{
    demo1_input_databuf_t *db  = (demo1_input_databuf_t *)args->obuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    /* Main loop */
    int i, rv,input;
    uint64_t mcnt = 0;
    int block_idx = 0;
    int a,b;

    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "waiting");
        hputi4(st.buf, "NETBKOUT", block_idx);
	hputi8(st.buf,"NETMCNT",mcnt);
        hashpipe_status_unlock_safe(&st);
	sleep(1); 
        // Wait for data
        /* Wait for new block to be free, then clear it
         * if necessary and fill its header with new values.
         */
        while ((rv=demo1_input_databuf_wait_free(db, block_idx)) 
                != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }

        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "receiving");
        hashpipe_status_unlock_safe(&st);
	
	//input two numbers 
	printf("Please input first number:");
	input=scanf("%d",&a);
	printf("Please input second number:");
	input=scanf("%d",&b);
	//a=10;
	//b=1;
	sleep(1);
	
	//move these two numbers to buffer
	db->block[block_idx].header.mcnt = mcnt;  
	db->block[block_idx].number1=a;
        db->block[block_idx].number2=b;

	//display a and b 
	hashpipe_status_lock_safe(&st);
	hputi8(st.buf,"A",a);
	hputi8(st.buf,"B",b);
	hashpipe_status_unlock_safe(&st);
        // Mark block as full
        demo1_input_databuf_set_filled(db, block_idx);

        // Setup for next block
        block_idx = (block_idx + 1) % db->header.n_block;
        mcnt++;

        /* Will exit if thread has been cancelled */
        pthread_testcancel();
    }

    // Thread success!
    return THREAD_OK;
}

static hashpipe_thread_desc_t demo1_net_thread = {
    name: "demo1_net_thread",
    skey: "NETSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {NULL},
    obuf_desc: {demo1_input_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&demo1_net_thread);
}
