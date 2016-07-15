#include "ini.h"
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <string.h>

#define MAXLINELENGTH 1000


static void ini_setStringValue(char** string,const char* value)
{
	int size;
	
	if (*string!=NULL) free(*string);

	if (value==NULL)
	{
		*string=NULL;
	}
	else
	{
		size=sizeof(char)*(strlen(value)+1);
		*string=malloc(size);
		memcpy(*string,value,size);
	}
}

static ini* ini_createIni()
{
    ini* dictionary;

    dictionary=malloc(sizeof(ini));
    dictionary->count=0;
    dictionary->currentAllocation=5;
    dictionary->items=malloc(sizeof(section)*dictionary->currentAllocation);

    return dictionary;
}

static section* ini_addSection(ini* ini,const char* name)
{
	section* newSection;
    section* currentSection;
	int size;
	
    if (ini->currentAllocation==ini->count)
    {
        ini->currentAllocation+=5;
        newSection=realloc(ini->items,sizeof(section)*ini->currentAllocation);
        if (newSection==NULL) perror("Not enough memory");
        else ini->items=newSection;
    }

    currentSection=&ini->items[ini->count];
    currentSection->count=0;
    currentSection->currentAllocation=5;
    currentSection->items=malloc(sizeof(keyValuePair)*currentSection->currentAllocation);
	currentSection->name=NULL;

	ini_setStringValue(&currentSection->name,name);
	
    ini->count++;

    return currentSection;
}

static keyValuePair* ini_addKeyValuePair(section* section,const char* key)
{
    keyValuePair* newSection;
    keyValuePair* newKeyValuePair;
	
	if (section->currentAllocation==section->count)
    {
        section->currentAllocation+=5;
        newSection=realloc(section->items,sizeof(keyValuePair)*section->currentAllocation);
        if (newSection==NULL) perror("Not enough memory");
        else section->items=newSection;
    }

    newKeyValuePair=&section->items[section->count];
	newKeyValuePair->key=NULL;
	newKeyValuePair->value=NULL;
	
	ini_setStringValue(&newKeyValuePair->key,key);

    section->count++;

    return newKeyValuePair;
}




void ini_free(ini* ini)
{
    int t;
    int s;
    section currentSection;
    keyValuePair currentKeyValuePair;

    for(t=0;t<ini->count;t++)
    {
        currentSection=ini->items[t];
        if (currentSection.name!=NULL) free(currentSection.name);
        for(s=0;s<currentSection.count;s++)
        {
            currentKeyValuePair=currentSection.items[s];
            free(currentKeyValuePair.key);
            free(currentKeyValuePair.value);
        }
        free(currentSection.items);
    }

    free(ini->items);
    free(ini);
}
void dump_ini(ini* ini)
{
    int t;
    int s;
    section currentSection;
    keyValuePair currentKeyValuePair;

    for(t=0;t<ini->count;t++)
    {
        currentSection=ini->items[t];
		if (currentSection.name==NULL)
		{
			printf("default\n");
		}
		else
		{
			printf("[%s]\n",currentSection.name);
		}
        for(s=0;s<currentSection.count;s++)
        {
            currentKeyValuePair=currentSection.items[s];
            printf("%s=%s\n",currentKeyValuePair.key,currentKeyValuePair.value);
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
    section* currentSection;
    keyValuePair* currentKeyValuePair;
	char value[MAXLINELENGTH];
	
    file = fopen(fileName, "r");
    if (file==NULL)
    {
        perror("Cannot open ini file");
        return NULL;
    }

    dictionary=ini_createIni();

    regcomp(&emptyRegex,"^[ \t\n]*$",REG_EXTENDED | REG_NOSUB );
    regcomp(&commentRegex,"^[ \t\n]*#",REG_EXTENDED | REG_NOSUB);
    regcomp(&sectionRegex,"^[ \t\n]*\\[([^]#]+)\\][ \t\n]*(#.*)?$",REG_EXTENDED);
    regcomp(&keyRegex,"^[ \t\n]*([^= \t]+)[ \t\n]*=[ \t\n]*((\"[^\"]*\")|([^# \t\n]+))[ \t\n]*(#.*)?$",REG_EXTENDED);

    matches = malloc (sizeof (*matches) * 5);
    line=malloc(sizeof(*line)*MAXLINELENGTH);

    currentSection=ini_addSection(dictionary,NULL);

    while(fgets(line, MAXLINELENGTH, file)!=NULL)
    {
        if (regexec(&emptyRegex, line, 0, NULL, 0)==0) continue;    // empty line
        if (regexec(&commentRegex, line, 0, NULL, 0)==0) continue;    // comment line
        if (regexec(&sectionRegex, line, 5, matches, 0)==0)    // new section
        {
            start = matches[1].rm_so;
            end = matches[1].rm_eo;
            length = end - start;

			strncpy (value, line+start, length);value[length]=0;
            currentSection=ini_addSection(dictionary,value);

            continue;
        }
        if (regexec(&keyRegex, line, 5, matches, 0)==0)    // new keyvalue pair
        {

            start = matches[1].rm_so;
            end = matches[1].rm_eo;
            length = end - start;

			strncpy (value, line+start, length);value[length]=0;
            currentKeyValuePair=ini_addKeyValuePair(currentSection,value);

            start = matches[2].rm_so;
            end = matches[2].rm_eo;
            if (line[start]=='"') { start++;end--;}
            length = end - start;
			if (length>0)
			{
				strncpy (value, line+start, length);value[length]=0;
				ini_setStringValue(&(currentKeyValuePair->value),value);
			}
			
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

section* ini_getSection(ini* ini, const char* sectionName)
{
    int t;
    section* currentSection;

    for(t=0;t<ini->count;t++)
    {
        currentSection=&ini->items[t];
        if ((currentSection->name==sectionName) ||  (strcmp(currentSection->name,sectionName)==0)) return currentSection;
    }
	
	currentSection=ini_addSection(ini,sectionName);

    return currentSection;
}
keyValuePair* ini_getKeyValuePair(section *section, const char* key,char* defaultValue)
{
    int t;
    keyValuePair* currentKeyValuePair;

    for(t=0;t < section->count;t++)
    {
        currentKeyValuePair=&section->items[t];
        if (strcmp(currentKeyValuePair->key,key)==0) return currentKeyValuePair;
    }

	
	currentKeyValuePair=ini_addKeyValuePair(section,key);
	ini_setStringValue(&currentKeyValuePair->value,defaultValue);

    return currentKeyValuePair;
}

char* ini_getString(ini *ini, const char* sectionName,const char* key,char* defaultValue)
{
    section* currentSection;
    keyValuePair* currentKeyValuePair;

    currentSection=ini_getSection(ini,sectionName);
    currentKeyValuePair=ini_getKeyValuePair(currentSection,key,defaultValue);

    return currentKeyValuePair->value;
}

int ini_getInt(ini *ini, const char* sectionName,const char* key,int defaultValue)
{
    section* currentSection;
    keyValuePair* currentKeyValuePair;
    int result;
	char defaultString[255];
	
	sprintf(defaultString, "%d", defaultValue);
	
    currentSection=ini_getSection(ini,sectionName);
    
	currentKeyValuePair=ini_getKeyValuePair(currentSection,key,defaultString);

    if (sscanf(currentKeyValuePair->value, "%d", &result)==1) return result;
    else return defaultValue;

}
unsigned short ini_getUnsignedShort(ini *ini, const char* sectionName,const char* keyName,unsigned short defaultValue)
{
    section* currentSection;
    keyValuePair* currentKeyValuePair;
    unsigned short result;
	char defaultString[255];
	
	sprintf(defaultString, "%hu", defaultValue);

    currentSection=ini_getSection(ini,sectionName);
    
	currentKeyValuePair=ini_getKeyValuePair(currentSection,keyName,defaultString);

    if (sscanf(currentKeyValuePair->value, "%hu", &result)==1) return result;
    else return defaultValue;

}

void ini_setString(ini *ini,const char* sectionName,const char* key,const char* value)
{
	section* currentSection;
    keyValuePair* currentKeyValuePair;

    currentSection=ini_getSection(ini,sectionName);
    currentKeyValuePair=ini_getKeyValuePair(currentSection,key,NULL);

	ini_setStringValue(&currentKeyValuePair->value,value);
}
