#ifndef _INIFILE_
#define _INIFILE_

typedef struct keyValuePair keyValuePair;
struct keyValuePair
{
   char* key;
   char* value;
};


typedef struct section section;
struct section
{
    char* name;
    int currentAllocation;
    int count;
    keyValuePair *items;
};

typedef struct ini ini;
struct ini
{
    int currentAllocation;
    int count;
    section *items;
};

ini* ini_open(const char* fileName);
void ini_free(ini* ini);
void ini_dump(ini* ini);
char* ini_getString(ini *ini, const char* section,const char* key,char* defaultValue);
int ini_getInt(ini *ini, const char* section,const char* key,int defaultValue);
unsigned short ini_getUnsignedShort(ini *ini, const char* sectionName,const char* keyName,unsigned short defaultValue);
section* ini_getSection(ini *ini, const char* name);
keyValuePair* ini_getKeyValuePair(section *section, const char* name);




#endif // _INIFILE_
