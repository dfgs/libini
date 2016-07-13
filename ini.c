#include "ini.h"
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#define MAXLINELENGTH 1000

static ini* createIni()
{
    ini* ini;

    ini=malloc(sizeof(ini));
    ini->count=0;
    ini->currentAllocation=5;
    ini->items=malloc(sizeof(section)*ini->currentAllocation);

    return ini;
}

static section* addSection(ini* ini)
{
	section* newSection;
    section* section;
	
    if (ini->currentAllocation==ini->count)
    {
        ini->currentAllocation+=5;
        newSection=realloc(ini->items,sizeof(section)*ini->currentAllocation);
        if (newSection==NULL) perror("Not enough memory");
        else ini->items=newSection;
    }

    section=&ini->items[ini->count];
    section->count=0;
    section->currentAllocation=5;
    section->items=malloc(sizeof(keyValuePair)*section->currentAllocation);
    ini->count++;

    return section;
}

static keyValuePair* addKeyValuePair(section* section)
{
    keyValuePair* newSection;
    keyValuePair* keyValuePair;
	
	if (section->currentAllocation==section->count)
    {
        section->currentAllocation+=5;
        newSection=realloc(section->items,sizeof(keyValuePair)*section->currentAllocation);
        if (newSection==NULL) perror("Not enough memory");
        else section->items=newSection;
    }

    keyValuePair=&section->items[section->count];
    section->count++;

    return keyValuePair;
}

void ini_free(ini* ini)
{
    int t;
    int s;
    section section;
    keyValuePair keyValuePair;

    for(t=0;t<ini->count;t++)
    {
        section=ini->items[t];
        free(section.name);
        for(s=0;s<section.count;s++)
        {
            keyValuePair=section.items[s];
            free(keyValuePair.key);
            free(keyValuePair.value);
        }
        free(section.items);
    }

    free(ini->items);
    free(ini);
}
void dump_ini(ini* ini)
{
    int t;
    int s;
    section section;
    keyValuePair keyValuePair;

    for(t=0;t<ini->count;t++)
    {
        section=ini->items[t];
        printf("[%s]\n",section.name);
        for(s=0;s<section.count;s++)
        {
            keyValuePair=section.items[s];
            printf("%s=%s\n",keyValuePair.key,keyValuePair.value);
         }
        printf("\n");
    }


}

ini* ini_open(const char* fileName)
{
    FILE *file;
    char* line;
    regex_t commentRegex,emptyRegex,sectionRegex,keyRegex;
    regmatch_t *matches = NULL;
    int start,end,length;
    ini* dictionary;
    section* currentsection;
    keyValuePair* keyValuePair;

    file = fopen(fileName, "r");
    if (file==NULL)
    {
        perror("Cannot open ini file");
        return NULL;
    }

    dictionary=createIni();

    regcomp(&emptyRegex,"^[ \t\n]*$",REG_EXTENDED | REG_NOSUB );
    regcomp(&commentRegex,"^[ \t\n]*#",REG_EXTENDED | REG_NOSUB);
    regcomp(&sectionRegex,"^[ \t\n]*\\[([^]#]+)\\][ \t\n]*(#.*)?$",REG_EXTENDED);
    regcomp(&keyRegex,"^[ \t\n]*([^= \t]+)[ \t\n]*=[ \t\n]*((\"[^\"]*\")|([^# \t\n]+))[ \t\n]*(#.*)?$",REG_EXTENDED);

    matches = malloc (sizeof (*matches) * 5);
    line=malloc(sizeof(*line)*MAXLINELENGTH);


    currentsection=addSection(dictionary);
    currentsection->name=malloc(sizeof(char)*8);
    strncpy (currentsection->name, "default", 7);
    currentsection->name[7]=0;

    while(fgets(line, MAXLINELENGTH, file)!=NULL)
    {
        if (regexec(&emptyRegex, line, 0, NULL, 0)==0) continue;    // empty line
        if (regexec(&commentRegex, line, 0, NULL, 0)==0) continue;    // comment line
        if (regexec(&sectionRegex, line, 5, matches, 0)==0)    // new section
        {
            start = matches[1].rm_so;
            end = matches[1].rm_eo;
            length = end - start;

            currentsection=addSection(dictionary);
            currentsection->name=malloc(sizeof(char)*length);
            strncpy (currentsection->name, line+start, length);
            currentsection->name[length]=0;

            //printf("New section: %s\n",currentsection.name);
            continue;
        }
        if (regexec(&keyRegex, line, 5, matches, 0)==0)    // new keyvalue pair
        {
            keyValuePair=addKeyValuePair(currentsection);

            start = matches[1].rm_so;
            end = matches[1].rm_eo;
            length = end - start;
            keyValuePair->key=malloc(length+1);
            strncpy (keyValuePair->key, line+start, length);
            keyValuePair->key[length]=0;


            start = matches[2].rm_so;
            end = matches[2].rm_eo;
            if (line[start]=='"') { start++;end--;}
            length = end - start;
            keyValuePair->value=malloc(length+1);
            if (length>0) strncpy (keyValuePair->value, line+start, length);
            keyValuePair->value[length]=0;

            //printf("New key value pair: %s / %s\n",keyValuePair.key,keyValuePair.value);
            continue;
        }

        fprintf(stderr,"Syntax error in ini file: %s",line);
        //fprintf(stderr,line);
    }

    fclose(file);

    free(matches);
    free(line);

    regfree(&keyRegex);
    regfree(&sectionRegex);
    regfree(&commentRegex);
    regfree(&emptyRegex);

    return dictionary;

}

section* ini_getSection(ini *ini, const char* name)
{
    int t;
    section* section;

    for(t=0;t<ini->count;t++)
    {
        section=&ini->items[t];
        if (strcmp(section->name,name)==0) return section;
    }

    return NULL;
}
keyValuePair* ini_getKeyValuePair(section *section, const char* name)
{
    int t;
    keyValuePair* keyValuePair;

    for(t=0;t<section->count;t++)
    {
        keyValuePair=&section->items[t];
        if (strcmp(keyValuePair->key,name)==0) return keyValuePair;
    }

    return NULL;
}

char* ini_getString(ini *ini, const char* sectionName,const char* keyName,char* defaultValue)
{
    section* section;
    keyValuePair* keyValuePair;

    section=ini_getSection(ini,sectionName);
    if (section==NULL) return defaultValue;
    keyValuePair=ini_getKeyValuePair(section,keyName);
    if (keyValuePair==NULL) return defaultValue;

    return keyValuePair->value;

}

int ini_getInt(ini *ini, const char* sectionName,const char* keyName,int defaultValue)
{
    section* section;
    keyValuePair* keyValuePair;
    int result;

    section=ini_getSection(ini,sectionName);
    if (section==NULL) return defaultValue;
    keyValuePair=ini_getKeyValuePair(section,keyName);
    if (keyValuePair==NULL) return defaultValue;

    if (sscanf(keyValuePair->value, "%d", &result)==1) return result;
    else return defaultValue;

}
unsigned short ini_getUnsignedShort(ini *ini, const char* sectionName,const char* keyName,unsigned short defaultValue)
{
    section* section;
    keyValuePair* keyValuePair;
    unsigned short result;

    section=ini_getSection(ini,sectionName);
    if (section==NULL) return defaultValue;
    keyValuePair=ini_getKeyValuePair(section,keyName);
    if (keyValuePair==NULL) return defaultValue;

    if (sscanf(keyValuePair->value, "%hu", &result)==1) return result;
    else return defaultValue;

}
