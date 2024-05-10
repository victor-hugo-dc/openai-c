#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }

    openai->verbose = true;

    Translation* response = translate(openai, "../output.mp3");
    if (response == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    if (response->error != NULL) {
        printf("%s\n", response->error->message);
        goto Exit;
    }
 
    printf("%s\n", response->text);

    if (openai->verbose) {
        printf("task: %s\n", response->task);
        printf("language: %s\n", response->language);
        for (size_t i = 0; i < response->segments_size; i++) {
            printf("text: %s\n", response->segments[i]->text);
            for (size_t j = 0; j < response->segments[i]->tokens_size; j++) {
                printf("%d ", response->segments[i]->tokens[j]);
            }
        }
    }

Exit:
    destroyClient(openai);
    destroyTranslation(response);
    return EXIT_SUCCESS;
}
