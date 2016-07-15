#ifndef _INIFILE_
#define _INIFILE_

typedef struct 
{
   char* key;
   char* value;
} keyValuePair;


typedef struct
{
    char* name;
    int currentAllocation;
    int count;
    keyValuePair *items;
} section; 


typedef struct 
{
    int currentAllocation;
    int count;
    section *items;
} ini;

ini* ini_open(const char* fileName);
void ini_free(ini* ini);
void ini_dump(ini* ini);

char* ini_getString(ini *ini, const char* sectionName,const char* key,char* defaultValue);
int ini_getInt(ini *ini, const char* sectionName,const char* key,int defaultValue);
unsigned short ini_getUnsignedShort(ini *ini, const char* sectionName,const char* key,unsigned short defaultValue);

void ini_setString(ini *ini,const char* sectionName,const char* key,const char* value);


section* ini_getSection(ini *ini, const char* nameName);
keyValuePair* ini_getKeyValuePair(section *section, const char* key,char* defaultValue);




#endif // _INIFILE_
