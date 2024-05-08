#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY", "Organization", "Project");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }

    Transcription* transcription = transcribe(openai, "../output.mp3");
    if (transcription == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    printf("%s\n", transcription->text);

    destroyClient(openai);
    destroyTranscription(transcription);

    return EXIT_SUCCESS;
}