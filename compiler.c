#include "data.h"
/* Shared global State variables*/

extern int firstRunParsing(FILE *fp, char *filename);
extern Bool parseSingleLine(char *line, FILE *fp);
void createExpandedSourceFile(FILE *source, FILE *target, char *fileName);

extern void initTablesArrays();
extern void printBinaryImg();
extern void initMemory();
extern void resetMemory();
extern void updateFinalCountersValue();
extern void printMemoryImgInRequiredObjFileFormat();
extern void parseAssemblyCode(FILE *fp);
extern void exportFilesMainHandler(char *baseFileName);
extern void initExternalOperandsList();

extern State getGlobalState();
extern void updateGlobalState(State new);
extern void setFileNamePath(char *s);

void handleSingleSourceFile(char *arg);

int main(int argc, char *argv[])
{

    handleSourceFiles(argc, argv);

    return 0;
}

int handleSourceFiles(int argc, char *argv[])
{

    int filesCount = argc - 1;
    int i = 1;
    if (filesCount < 1)
    {
        yieldError(AssemblerDidNotGetSourceFiles);
        exit(1);
    }

    while (--argc)
    {
        updateGlobalState(parsingMacros);
        handleSingleSourceFile(argv[i]);
        i++;
    }

    return True;
}

void handleSingleSourceFile(char *arg)
{
    State (*globalState)() = &getGlobalState;
    FILE *fptr, *expandedSrc;
    char *fileName = calloc(strlen(arg) + 3, sizeof(char *));
    void (*setFileName)(char *) = &setCurrentFileName;
    void (*setPath)(char *) = &setFileNamePath;
    if (strstr(arg, "./"))
    {
        char *s;
        s = strrchr(arg, '/');
        s++;
        sscanf(s, "%s", fileName);
        (*setFileName)(strcat(fileName, ".as"));
        s = strchr(arg, '.');
        (*setPath)(s);
    }
    else
    {
        (*setPath)(strcat(arg, ".as"));
    }
    sscanf(arg, "%s", fileName);

    if ((fptr = fopen(strcat(fileName, ".as"), "r")) == NULL)
    {
        yieldError(fileCouldNotBeOpened);
        return;
    }
    arg[strlen(arg) - 1] = 'm';
    expandedSrc = fopen(arg, "w+");
    if (expandedSrc == NULL)
    {
        fclose(fptr);
        /*         updateGlobalState(goToNextFileOrEndProgram); */
    }
    else
    {
        initTablesArrays();
        createExpandedSourceFile(fptr, expandedSrc, fileName);
        rewind(expandedSrc);
        printMacroTable();

        if ((*globalState)() == firstRun)
        {
            /*    initTablesArrays(); */
            parseAssemblyCode(expandedSrc);
            if ((*globalState)() == secondRun)
            {
                rewind(expandedSrc);
                updateFinalCountersValue();
                printSymbolTable();
                initMemory();
                if (areExternalsExist())
                    initExternalOperandsList();
                parseAssemblyCode(expandedSrc);

                if ((*globalState)() == exportFiles)
                {
                    exportFilesMainHandler(arg);
                    resetMemory();
                }
                else
                    printf("\nSecond Run Finished With Errors, files will not be exported!\n");
            }
        }
        else
            printf("failed to create new .am (expanded source code) file for the %s source file\nmoving on to the next file if exist", fileName);
    }

    free(fileName);
    /*     fclose(fptr); */
    fclose(expandedSrc);
}