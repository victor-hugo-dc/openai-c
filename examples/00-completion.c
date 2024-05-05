#include <stdio.h>
#include <stdlib.h>
#include "openai.h"

int main(void) {
    Client* openai = OpenAI("MY-API-KEY", "Organization", "Project");
    if (openai == NULL) {
        return EXIT_FAILURE;
    }
    
    Completion *response = chat(openai, "gpt-3.5-turbo-0125", "Hello world", 0.7);
    if (response == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    printf("%s\n", response->choices[0]->message->content);
    
    destroyCompletion(response);
    destroyClient(openai);
    
    return EXIT_SUCCESS;
}
