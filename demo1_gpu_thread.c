/*demo1_gpu_thread.c
 *
 * Performs spectroscopy of incoming data using demo1GPU
 */

#include <stdio.h>
#include <stdlib.h>
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
    // Local aliases to shorten access to args fields
    demo1_input_databuf_t *db_in = (demo1_input_databuf_t *)args->ibuf;
    demo1_output_databuf_t *db_out = (demo1_output_databuf_t *)args->obuf;
    hashpipe_status_t st = args->st;
    const char * status_key = args->thread_desc->skey;

    int rv,a,b,c;
    uint64_t mcnt=0;
    int curblock_in=0;
    int curblock_out=0;
    while (run_threads()) {

        hashpipe_status_lock_safe(&st);
        hputi4(st.buf, "GPUBLKIN", curblock_in);
        hputs(st.buf, status_key, "waiting");
        hputi4(st.buf, "GPUBKOUT", curblock_out);
	hputi8(st.buf,"GPUMCNT",mcnt);
        hashpipe_status_unlock_safe(&st);
	sleep(1);
        // Wait for new input block to be filled
        while ((rv=demo1_input_databuf_wait_filled(db_in, curblock_in)) != HASHPIPE_OK) {
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

        // Got a new data block, update status and determine how to handle it
        /*hashpipe_status_lock_safe(&st);
        hputu8(st.buf, "GPUMCNT", db_in->block[curblock_in].header.mcnt);
        hashpipe_status_unlock_safe(&st);*/

        // Wait for new output block to be free
        while ((rv=demo1_output_databuf_wait_free(db_out, curblock_out)) != HASHPIPE_OK) {
            if (rv==HASHPIPE_TIMEOUT) {
                hashpipe_status_lock_safe(&st);
                hputs(st.buf, status_key, "blocked gpu out");
                hashpipe_status_unlock_safe(&st);
                continue;
            } else {
                hashpipe_error(__FUNCTION__, "error waiting for free databuf");
                pthread_exit(NULL);
                break;
            }
        }

        // Note processing status
        hashpipe_status_lock_safe(&st);
        hputs(st.buf, status_key, "processing gpu");
        hashpipe_status_unlock_safe(&st);

	//calculate c=a+b
	a=db_in->block[curblock_in].number1;
	b=db_in->block[curblock_in].number2;
	c=a+b;
	sleep(1);
        db_out->block[curblock_out].sum=c;
	
        // Mark output block as full and advance
        demo1_output_databuf_set_filled(db_out, curblock_out);
        curblock_out = (curblock_out + 1) % db_out->header.n_block;

        // Mark input block as free and advance
        demo1_input_databuf_set_free(db_in, curblock_in);
        curblock_in = (curblock_in + 1) % db_in->header.n_block;
	mcnt++;
	//display sum in status
	hashpipe_status_lock_safe(&st);
	hputi4(st.buf,"GPUSUM",c);
	hashpipe_status_unlock_safe(&st);
        /* Check for cancel */
        pthread_testcancel();
    }
    return NULL;
}

static hashpipe_thread_desc_t demo1_gpu_thread = {
    name: "demo1_gpu_thread",
    skey: "GPUSTAT",
    init: NULL,
    run:  run,
    ibuf_desc: {demo1_input_databuf_create},
    obuf_desc: {demo1_output_databuf_create}
};

static __attribute__((constructor)) void ctor()
{
  register_hashpipe_thread(&demo1_gpu_thread);
}

