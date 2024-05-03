#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "cJSON.h"

// TODO: Add ways to make the ClientOptions useful and configurable
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

// TODO: Add ways of having a conversation with OpenAI rather than it being one message
typedef struct {
    char* apiKey;
    char* organization;
    char* project;
    ClientOptions* _options;

} Client;

typedef struct {
    char* ptr;
    size_t length;

} String;

typedef struct {
    char* role;
    char* content;

} Message;

typedef struct {
    int index;
    Message* message;
    char* logprobs;
    char* finish_reason;

} Choices;

typedef struct {
    int prompt_tokens;
    int completion_tokens;
    int total_tokens;

} Usage;

typedef struct {
    char* message;
    char* type;
    char* param;
    char* code;
} Error;

typedef struct {
    char* id;
    char* object;
    long created;
    char* model;

    Usage* usage;
    
    Choices** choices;
    int choices_count;
    
    char* system_fingerprint;
    Error* error;

} Response;

Client* OpenAI(char* apiKey, char* organization, char* project);
void destroyClient(Client* object);

Response* chat(Client* openai, const char* model, const char* messages, float temperature); 
void destroyResponse(Response* response);

void destroyMessage(Message* message);

void destroyChoicesList(Choices** choicesList, int count);
void destroyChoices(Choices* choices);
