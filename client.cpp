#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "FIFOreqchannel.h"
#include <thread>
#include <time.h>

using namespace std;


FIFORequestChannel* create_new_channel(FIFORequestChannel* mainchan){
    char name [1024];
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    mainchan->cwrite(&m, sizeof(m));
    mainchan->cread(name, 1024);
    FIFORequestChannel* newchan = new FIFORequestChannel(name, FIFORequestChannel::CLIENT_SIDE);
    return newchan;
}

void patient_thread_function(int n, int pno, BoundedBuffer* request_buffer){
    /* What will the patient threads do? */
    datamsg d (pno, 0.0, 1);
    double response = 0;
    for(int i = 0; i < n; i++){
        //chan->cwrite(&d, sizeof(d));
        //chan->cread(&response, sizeof(double));
        //hc->update(pno, response);
        request_buffer->push((char *)&d, sizeof(datamsg));
        d.seconds += 0.004;
    }
}

void worker_thread_function(FIFORequestChannel* chan, BoundedBuffer *request_buffer ,HistogramCollection* hc){
    /*
		Functionality of the worker threads	
    */
   while(true){
       
   }
}



int main(int argc, char *argv[])
{
    int n = 100;    //default number of requests per "patient"
    int p = 10;     // number of patients [1,15]
    int w = 100;    //default number of worker threads
    int b = 20; 	// default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the message buffer
    srand(time_t(NULL));
    
    
    int pid = fork();
    if (pid == 0){
		// modify this to pass along m
        execl ("server", "server", (char *)NULL);
    }
    
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
	HistogramCollection hc;

    for(int i = 0; i < p; i++){
        Histogram *h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }

    // making worker channels
    FIFORequestChannel* wchans[w];
    for(int i = 0; i < w; i++){
        wchans[i] = create_new_channel(chan);
    }
	
	
	
    struct timeval start, end;
    gettimeofday (&start, 0);

    /* Start all threads here */
    thread patient [p];
    for(int i = 0; i < p; i++){
        patient [i] = thread(patient_thread_function, n, i+1, &request_buffer);
    }

    //worker threads
    thread workers[w];
    for(int i = 0; i < p; i++){
        workers [i] = thread(worker_thread_function, wchans[i], &request_buffer, &hc);
    }
	
    //joining threads
    for(int i = 0; i < p; i++){
        patient [i].join();
    }

	/* Join all threads here */
    gettimeofday (&end, 0);
    //timediff (start, end);
    // print the results
	hc.print ();


    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!!!" << endl;
    delete chan;
    
}
