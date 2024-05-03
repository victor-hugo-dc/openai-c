#include <stdio.h>
#include <stdlib.h>
#include "openai.h"

int main(void)
{
    Client *openai = OpenAI("MY-API-KEY", "Personal", "Default Project");
    Response *response = chat(openai, "gpt-3.5-turbo-0125", "Hello world", 0.7);
    if (response == NULL) {
        destroyClient(openai);
        return EXIT_FAILURE;
    }

    printf("%s\n", response->id);
    printf("%s\n", response->object);
    printf("%s\n", response->model);
    printf("%s\n", response->choices[0]->message->content);
    printf("%s\n", response->system_fingerprint);
    printf("%d\n", response->usage->completion_tokens);

    destroyResponse(response);
    destroyClient(openai);
    return EXIT_SUCCESS;
}
