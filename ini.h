#ifndef _INIFILE_
#define _INIFILE_

#ifdef __cplusplus
extern "C"{
#endif 

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
} Section; 


typedef struct 
{
    int currentAllocation;
    int count;
    Section *items;
} IniFile;



IniFile* ini_open(const char* fileName);
void ini_free(IniFile* ini);
void ini_dump(IniFile* ini);

char* ini_getString(IniFile* ini, const char* sectionName,const char* key,const char* defaultValue);
int ini_getInt(IniFile* ini, const char* sectionName,const char* key,int defaultValue);
unsigned short ini_getUnsignedShort(IniFile* ini, const char* sectionName,const char* key,unsigned short defaultValue);

void ini_setString(IniFile *ini,const char* sectionName,const char* key,const char* value);


Section* ini_getSection(IniFile *ini, const char* sectionName);
keyValuePair* ini_getKeyValuePair(Section* section, const char* key,const char* defaultValue);



#ifdef __cplusplus
}
#endif


#endif // _INIFILE_
