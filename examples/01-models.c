#include <stdio.h>
#include <stdlib.h>
#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY", "Organization", "Project");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }

    ModelList* response = models(openai);
    if (response == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < response->model_count; i++) {
        printf("%s\n", response->data[i]->id);

    }
    
    destroyClient(openai);
    destroyModels(response);
}
