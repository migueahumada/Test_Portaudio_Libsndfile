// Test_Portaudio_Libsndfile.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <thread>
#include <sndfile.h>
#include <portaudio.h>

#define SAMPLE_RATE 96000
#define FRAMES_PER_BUFFER 512

/// <summary>
/// Column for handling the data from a soundfile with the libsndfile library
/// </summary>
typedef struct SoundData 
{
    SF_INFO info;
    SNDFILE* soundfile;
}SoundData;

static void customThreadFunction(const char* message) 
{
    printf("%s\n", message);
}

/// <summary>
/// Callback that plays audio from an audio file
/// </summary>
static int myCallback(const void* input, void* output, unsigned long frameCount,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags, void* userData)
{
    /*
        Cast all void parameters
        - user Data is cast to sound data to use its properties
        - output is cast to float, cuz we'll work with float instead of bytes
        - input is cast to void because we don't need it right now
        Initialize read samples, which keeps track of the read samples
        of a file.
    */
    SoundData* data = (SoundData*)userData;
    float* out = (float*)output;
    (void)input;
    
    // Calculate total number of requested samples (frames * channels)
    unsigned long totalSamples = frameCount * data->info.channels;

    // Read up to `totalSamples` samples from the file
    sf_count_t readSamples = sf_read_float(data->soundfile, out, totalSamples);

    // Zero out remaining buffer if fewer samples were read
    if (readSamples < totalSamples) {
        // Start zeroing at the index where `sf_read_float` stopped
        for (sf_count_t i = readSamples; i < totalSamples; i++) {
            out[i] = 0.0f;  // Zero out the remaining portion of the buffer
        }
        return paComplete; // Signal that playback is complete
    }

    return paContinue;
    
    /*
    sf_count_t readSamples;

    //We need to flush the data each time
    memset(out,0,sizeof(float)*frameCount*data->info.channels); 

    readSamples = sf_read_float(data->soundfile,out,frameCount*data->info.channels);

    if (readSamples < frameCount)
    {
        return paComplete;
    }

    return paContinue;
    */
}

int main()
{
    /*  
        Allocate memory for the sound data, because we won't know 
        the data during compile time, therefore we need to use dynamic 
        memory allocation. If not done we'll just exit.
    */

    SoundData* soundData = NULL; //Data from a sound
    soundData = (SoundData*)malloc(sizeof(SoundData));
    if (soundData == NULL) {
        puts("Couldn't allocate memory for SoundData.");
        return EXIT_FAILURE;
    }

    /*
        The SoundData struct holds a pointer to a soundfile and a
        soundfile info that stores all the info we need. So we call sfopen
        to give the soundfile a real file to reference.
    */
    soundData->soundfile = sf_open("D:/Coding/C++/Test_Portaudio_Libsndfile/assets/Audio_01.wav", SFM_READ, &soundData->info);
    if (soundData->soundfile == NULL) {
        //printf("Error: %s\n", sf_strerror(NULL));
        printf("Error: Couldn't open the soundfile.\n");
        free(soundData);
        return EXIT_FAILURE;
    }

    //DEBUG STUFF
    printf("The samplerate of the soundfile is: %d\n",soundData->info.samplerate);

    /* PORTAUDIO */

    PaError error; //To debug errors related to portaudio
    PaStream* stream = NULL; //Stream to playback audio from our output audio device

    /*
        We init portaudio which will help us to output audio.
    */
    error = Pa_Initialize();
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));
        return EXIT_FAILURE;
    }

    /*
        We use a default open function. For that we need a stream, 
        0 is for the default input device, 2 is for the default output device,
        the sample rate that we'll use for reading, the frames per buffer
        we'll for reading, a callback function, the soundData.
    */
    error = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32,
        SAMPLE_RATE, FRAMES_PER_BUFFER, myCallback, soundData);
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));
        Pa_Terminate();
        return EXIT_FAILURE;
    }
    
    //Start the stream
    error = Pa_StartStream(stream);
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));
        Pa_Terminate();
        return EXIT_FAILURE;
    }

    /*
        Put pa to sleep until the sound finishes playing.
        Its given in miliseconds so we do the operation
        frames/samplerate = duration of the file(seconds)
        and multiply by 1000 to convert it into miliseconds.
        
    */
    Pa_Sleep((long)(soundData->info.frames/SAMPLE_RATE)*1000);

    /*
        Once finished, we close the soundfile, free the sound data, cuz we
        don't need it anymore.
    */
    sf_close(soundData->soundfile);
    free(soundData);

    //Stop the stream
    error = Pa_StopStream(stream);
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));
        Pa_Terminate();
        return EXIT_FAILURE;
    }

    //Close the stream
    error = Pa_CloseStream(stream);
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));
        Pa_Terminate();
        return EXIT_FAILURE;
    }

    //DEBUG STUFF
    puts("Pure success brother!\n");

    /* Terminate the Portaudio */
    error = Pa_Terminate();
    if (error != paNoError) {
        printf("Error: %s ", Pa_GetErrorText(error));

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
    
}
