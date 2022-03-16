#include "data.h"
extern Bool yieldError(Error err);
extern char *trimFromLeft(char *s);
extern State globalState;
extern unsigned currentLine;
extern const char *regs[REGS_SIZE];

extern int countSpaceCharacters(char *s);
int getNumberLength(int num)
{
    int length = 1;
    /*counts the first digit of the given number*/
    while (num / 10) /*divides the number each time by 10 in order to remov digits its already counted*/
        length++;    /*increase the digit's counter by 1*/

    return length; /*returns the amount of digits in the given number*/
}
int countConsecutiveCommasAndTrimSpaces(char *s)
{
    int counter = 0;
    s = trimFromLeft(s);
    for (; s && *s == ','; counter++, s++)
        ;

    printf("s:%s counter:%d\n", s, counter);
    return counter;
}

int skipTokenNonDigitCharacters(char *s)
{
    int count = 0;
    for (; !isdigit(*s) && !isspace(*s) && *s != ','; s++, count++)
        ;
    return count;
}

Bool countAndVerifyDataArguments(char *line, char *token)
{
    /*
    token is only one  token at time connected to strtok, you can use it
    or you can use line argument which is the fullline without strtok,
    inside this function we create string variable named "args[MAX_LINE_LEN+1]"
    that takes the full string of arguments that starts straight after the .data
    token. choose what you want.
     */

    char args[MAX_LINE_LEN + 1] = {0};
    int size = 0, num = 0, n = 0, num2 = 0, n2 = 0, commasCounter = 0, i = 0, len = 0, skip = 0;
    char c = 0, *p = strstr(line, DATA) + strlen(DATA);
    Bool isValid = True;
    Bool minusOrPlusFlag = False;
    len = strlen(p);
    memcpy(args, p, len);
    p = args;
    p = trimFromLeft(p);
    i = len - strlen(p);

    if (*p == ',')
    {
        isValid = yieldError(illegalApearenceOfCommaBeforeFirstParameter);
        p++;
        i++;
    }

    while (p && i < len)
    {
        /*         skip = countSpaceCharacters(p);
                commasCounter += countConsecutiveCommasAndTrimSpaces(p);
                p += skip + commasCounter;
                i += skip + commasCounter; */
        /*         i = len - strlen(p); */

        printf("args[i]:%c\np:%c\nsize:%d commaCounter:%d num:%d c:%c\n\n\n", args[i], *p, size, commasCounter, num, c);

        if (!isdigit(p[0]) && !isspace(*p))
        {
            if (*p == '-' || *p == '+')
            {

                if (!minusOrPlusFlag)
                    minusOrPlusFlag = True;
                else
                    isValid = yieldError(illegalApearenceOfCharactersOnLine);
                p++;
                i++;
            }
            else if (*p != ',')
            {
                isValid = yieldError(illegalApearenceOfCharactersOnLine);
                skip = skipTokenNonDigitCharacters(p) + countSpaceCharacters(p);
                p += skip;
                i += skip;
                commasCounter += countConsecutiveCommasAndTrimSpaces(p);
                p += commasCounter;
                i += commasCounter;
                size++;
            }
            if (*p == ',')
            {
                commasCounter += countConsecutiveCommasAndTrimSpaces(p);
                skip = countSpaceCharacters(p);
                p += commasCounter + skip;
                i += commasCounter + skip;
                i = len - strlen(p);
            }
        }

        if (commasCounter < 1 && size > 0)
        {
            isValid = yieldError(wrongInstructionSyntaxMissinCommas);
            commasCounter = size;
        }

        else if (commasCounter > 0 && size < (commasCounter - 1))
        {
            isValid = yieldError(wrongInstructionSyntaxExtraCommas);
            commasCounter = size;
        }
        else if (size > commasCounter)
        {
            isValid = yieldError(wrongInstructionSyntaxMissinCommas);
            commasCounter = size;
        }

        if (isdigit(*p))
        {
            i = len - strlen(p);
            sscanf(&args[i], "%d%n%c%d", &num, &n, &c, &num2);
            if (c && c == ',')
                commasCounter++;
            else if (c && c != ',' && !isspace(c) && c != '.')
                isValid = yieldError(illegalApearenceOfCharactersOnLine);

            else if (c == '.')
            {
                /*           int len = getNumberLength(num2);
                          printf("extra float number length::%d\n", len); */
                isValid = yieldError(wrongArgumentTypeNotAnInteger);
                p += n;
                i += n;
                sscanf(&args[i], "%d%n", &num, &n);

                /*    n += getNumberLength(num2); */
            }
            size++;
            minusOrPlusFlag = False;
        }
        if (n)
        {
            p += n;
            i += n;
            /*             i += n; */
            c = n = num = num2 = n2 = 0;
        }
        else
        {
            p++;
            i++;
        }
    }

    if (commasCounter > size)
        isValid = yieldError(illegalApearenceOfCommaAfterLastParameter);

    if (isValid)
        increaseDataCounter(size);

    return isValid;
}

Bool countAndVerifyStringArguments(char *token)
{

    if (isInstruction(token))
        token = strtok(NULL, " \t \n");
    if (token[0] == '\"' && token[strlen(token) - 1] != '\"')
        return yieldError(closingQuotesForStringIsMissing);
    else if (token[0] != '\"')
        return yieldError(expectedQuotes);

    increaseDataCounter((int)(strlen(token) - 1)); /*counts the \0 at the end of the string as well*/

    return True;
}

ParseState handleState(char *token, char *line, ParseState state)

{

    switch (state)
    {
    case skipLine:
        return lineParsedSuccessfully;

    case newLine:
    {
        if (isComment(token))
            return lineParsedSuccessfully;

        if (isLabel(token))
            return skipToNextToken;

        else if (isInstruction(token))
        {
            if (globalState == firstRun)
                return handleInstruction(getInstructionType(token), token, strtok(NULL, " \t \n"), line) ? lineParsedSuccessfully : Err;
            else
            {
                int type = getInstructionType(token);
                token = strtok(NULL, ", \t \n");

                if (type == _TYPE_DATA)
                {
                    return writeDataInstruction(token);
                }
                else if (type == _TYPE_STRING)
                {
                    return writeStringInstruction(token);
                }
                else
                    return lineParsedSuccessfully;
            }
        }

        else if (isOperation(token))
        {
            char args[MAX_LINE_LEN] = {0};
            /*            line = line + strlen(token); */
            strcpy(args, (line + strlen(token)));
            return globalState == firstRun ? handleOperation(token, args) : writeOperationBinary(token, args);
        }
        else
        {
            if (globalState == firstRun)
            {
                yieldError(illegalLabelUseExpectedOperationOrInstruction);
                return Err;
            }
        }
    }

    default:
        break;
    }

    return lineParsedSuccessfully;
}

void parseSingleLine(char *line)
{
    ParseState state = newLine;
    char lineCopy[MAX_LINE_LEN] = {0};
    char *token;
    printf("\ninside parseSingleLine, Line Number (%d):\n%s\n", currentLine, line);
    memcpy(lineCopy, line, strlen(line));
    if (globalState == firstRun)
        token = strtok(lineCopy, " \t \n");
    else if (globalState == secondRun)
        token = strtok(lineCopy, ", \t \n");

    while (token != NULL && state != lineParsedSuccessfully)
    {
        state = handleState(token, line, state);
        switch (state)
        {
        case skipLine:
            state = lineParsedSuccessfully;
        case skipToNextToken:
        {

            if (globalState == firstRun)
                state = handleLabel(token, strtok(NULL, " \t \n"), line);
            else if (globalState == secondRun)
            {
                line = line + strlen(token) + 1;
                state = handleState(strtok(NULL, ", \t \n"), line, newLine);
            }

            break;
        }

        case Err:
        {
            break;
        }
        default:
            break;
        }

        if (globalState == firstRun)
            token = strtok(NULL, " \t \n");
        else if (globalState == secondRun)
            token = strtok(NULL, ", \t \n");
    }

    if (!state)
        globalState = collectErrors;

    currentLine++;
}

Bool parseFile(FILE *fp, char *filename)
{
    int c = 0;
    int i = 0;
    char line[MAX_LINE_LEN + 1] = {0};
    currentLine = 1;

    if (globalState == secondRun)
        printf("\n\n\nSecond Run:\n");
    else
        printf("\n\n\nFirst Run:\n");

    while (((c = fgetc(fp)) != EOF))
    {
        if (globalState != secondRun && (i >= MAX_LINE_LEN - 1 && c != '\n'))
        {

            globalState = collectErrors;
            yieldError(maxLineLengthExceeded);
            memset(line, 0, MAX_LINE_LEN);
            i = 0;
        }

        if (c == '\n')
        {

            parseSingleLine(line);
            memset(line, 0, MAX_LINE_LEN);
            i = 0;
        }

        else if (isspace(c))
            line[i++] = ' ';

        else
        {
            if (isprint(c))
                line[i++] = c;
        }
    }

    if (i > 0)
    {
        parseSingleLine(line);
        memset(line, 0, i);
    }

    return globalState != collectErrors ? True : False;
}
