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

} OpenAI;

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
    char* id;
    char* object;
    long created;
    char* model;

    Usage* usage;
    
    Choices** choices;
    int choices_count;
    
    char* system_fingerprint;

} Response;

// {
//     "error": {
//         "message": "Incorrect API key provided: OPENAIKEY. You can find your API key at https://platform.openai.com/account/api-keys.",
//         "type": "invalid_request_error",
//         "param": null,
//         "code": "invalid_api_key"
//     }
// }

typedef struct {
    char* message;
    char* type;
    char* param;
    char* code;
} Error;

OpenAI* createOpenAI(char* apiKey, char* organization, char* project);
void destroyOpenAI(OpenAI* object);

Response* chat(OpenAI* openai, const char* model, const char* messages, float temperature); 
void destroyResponse(Response* response);

Message* extractMessageFromJSON(cJSON* message);
void destroyMessage(Message* message);

Choices** extractChoicesFromJSON(cJSON* choices);
void destroyChoicesList(Choices** choicesList, int count);
void destroyChoices(Choices* choices);

Usage* extractUsageFromJSON(cJSON* usage);
void destroyUsage(Usage* usage);

Error* extractErrorFromJSON(cJSON* error);
void destroyError(Error* error);