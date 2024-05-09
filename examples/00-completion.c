#include <stdio.h>
#include <stdlib.h>
#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }
    
    // Adjust the GPT model and temperature accordingly
    openai->chat_model = GPT_4;
    openai->temperature = 0.7;

    Completion *response = chat(openai, "What is the tallest mountain in the world?");
    if (response == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    // Error handling
    if (response->error != NULL) {
        destroyCompletion(response);
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    printf("%s\n", response->choices[0]->message->content);
    
    destroyCompletion(response);
    destroyClient(openai);
    
    return EXIT_SUCCESS;
}
