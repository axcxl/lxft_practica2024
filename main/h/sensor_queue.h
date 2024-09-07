#ifndef SENSOR_QUEUE_H
#define SENSOR_QUEUE_H

#define SENSQ_LEN 40

#define str(x) #x
#define xstr(x) str(x)

#define FOREACH_SENSQTYPE(SENSQ_TYPE) \
    SENSQ_TYPE(INVALID) \
    SENSQ_TYPE(TEMP)    \
    SENSQ_TYPE(HUM)     \
    SENSQ_TYPE(PRES)    \
    SENSQ_TYPE(ENDTYPE) \

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,


enum sensq_type {
    FOREACH_SENSQTYPE(GENERATE_ENUM)
};

static const char *sensq_string[] = {
    FOREACH_SENSQTYPE(GENERATE_STRING)
};

typedef struct sensq
{
    float value;
    enum sensq_type type;
}sensq;


#endif /* SENSOR_QUEUE_H */