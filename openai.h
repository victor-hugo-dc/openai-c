#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
    char* apiKey;
    char* organization;
    char* project;
    char* baseURL;
    int timeout;
    // httpAgent
    // fetch
    int maxRetries;
    // defaultHeaders
    // defaultQuery
    bool dangerouslyAllowBrowser;

} ClientOptions;


typedef struct {
    char* apiKey;
    char* organization;
    char* project;
    ClientOptions* _options;

} OpenAI;

typedef struct {
    char* ptr;
    size_t length;
} String;

typedef struct {
    char* id;
    char* object;
    long created;
    char* model;

    int prompt_tokens;
    int completion_tokens;
    int total_tokens;

    int index;
    char* message_role;
    char* message_content;
    // bool message_logprobs; this is null sometimes
    char* finish_reason;

    char* system_fingerprint;
} Response;

OpenAI* createOpenAI(char* apiKey, char* organization, char* project);
void destroyOpenAI(OpenAI* object);
Response* chat(OpenAI* openai, const char* model, const char* messages, float temperature); 
void destroyResponse(Response* response);