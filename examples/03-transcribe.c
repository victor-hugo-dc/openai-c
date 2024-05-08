#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY", "Organization", "Project");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }

    openai->verbose = true;

    Transcription* transcription = transcribe(openai, "../output.mp3");
    if (transcription == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    if (transcription->error != NULL) {
        printf("%s\n", transcription->error->message);
        goto Exit;
    }
 
    printf("%s\n", transcription->text);

    if (openai->verbose) {
        Word* word;
        for (size_t i = 0; i < transcription->word_count; i++) {
            word = transcription->words[i];
            printf("%.2f - %.2f: %s\n", word->start, word->end, word->word);

        }
    }

Exit:
    destroyClient(openai);
    destroyTranscription(transcription);

    return EXIT_SUCCESS;
}