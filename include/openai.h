#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <cjson/cJSON.h>

enum GPT {
    GPT_3_5_TURBO_1106,
    GPT_3_5_TURBO_16k,
    GPT_4,
    GPT_4_TURBO_2024_04_09,
    GPT_4_0613,
    GPT_4_TURBO,
    GPT_4_VISION_PREVIEW,
    GPT_3_5_TURBO_INSTRUCT_0914,
    GPT_3_5_TURBO_INSTRUCT,
    GPT_4_1106_VISION_PREVIEW,
    GPT_4_0125_PREVIEW,
    GPT_4_TURBO_PREVIEW,
    GPT_3_5_TURBO_0125,
    GPT_3_5_TURBO,
    GPT_3_5_TURBO_0613,
    GPT_4_1106_PREVIEW,
    GPT_3_5_TURBO_16k_0613,
};

enum TTS {
    TTS_1,
    TTS_1_HD,
};

enum VOICE {
    ALLOY,
    ECHO,
    FABLE,
    ONYX,
    NOVA,
    SHIMMER,
};

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

    enum GPT chat_model;
    enum TTS speech_model;
    enum VOICE voice;

    double temperature;

    int verbose; // default to zero

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

} Completion;

Client* OpenAI(char* apiKey, char* organization, char* project);
void destroyClient(Client* object);

const char* getChatModelName(enum GPT model);

Completion* chat(Client* openai, const char* messages); 
void destroyCompletion(Completion* response);

void destroyMessage(Message* message);

void destroyChoicesList(Choices** choicesList, int count);
void destroyChoices(Choices* choices);

typedef struct {
    char* id;
    char* object;
    long created;
    char* owned_by;

} Model;

typedef struct {
    char* object;
    Model** data;
    int model_count;

} ModelList;

ModelList* models(Client* openai);
void destroyModels(ModelList* models);

void generate_speech(Client* openai, const char* message, const char* filename);
const char* getVoice(enum VOICE voice);
const char* getSpeechModel(enum TTS model);

typedef struct {
    char* word;
    double start;
    double end;
} Word;

typedef struct {
    char* task;
    char* language;
    double duration;
    Word** words;
    int word_count;

    char* text;
    Error* error;

} Transcription;

int valid_filename(const char *filename);
Transcription* transcribe(Client* openai, const char* filepath);
void destroyTranscription(Transcription* transcription);