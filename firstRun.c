#include "data.h"
extern Bool yieldError(Error err);
extern Bool yieldWarning(Warning err);

extern Operation *getOperationByName(char *s);
extern Bool addSymbol(char *name, unsigned value, unsigned isCode, unsigned isData, unsigned isEntry, unsigned isExternal);
extern Bool isLabelNameAlreadyTaken(char *name, ItemType type);
extern Bool verifyLabelNaming(char *s);
extern void increaseDataCounter(int amount);
extern void increaseInstructionCounter(int amount);
extern unsigned getDC();
extern unsigned getIC();
extern Bool writeOperationBinary(char *operationName, char *args);

/* parse.c */
extern Bool countAndVerifyDataArguments(char *line);
extern Bool countAndVerifyStringArguments(char *line);
extern void parseAssemblyCode(FILE *src);

extern Bool writeStringInstruction(char *s);
extern Bool writeDataInstruction(char *s);

ParseState handleOperation(char *operationName, char *args)
{
    Operation *p = getOperationByName(operationName);
    char first[MAX_LABEL_LEN] = {0}, second[MAX_LABEL_LEN] = {0};
    int commasCounter = 0;
    AddrMethodsOptions active[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    Bool areOperandsLegal = True;

    /*
    val1,val2
    val1, val2
    val1 ,val2
    val1 , val2
     */
    if (*args)
    {
        char *s1 = args, *s2;
        Bool parsedOperands = False;
        int counter = 0;
        s1 = trimFromLeft(args);

        if (*s1 == ',')
        {
            while (*s1 && (*s1 == ',' || isspace(*s1)))
                s1++;

            if (*s1)
                areOperandsLegal = yieldError(illegalApearenceOfCommaBeforeFirstParameter);
            else
                areOperandsLegal = yieldError(illegalApearenceOfCommaAfterLastParameter);
        }

        s2 = s1;
        while (*s1 && !parsedOperands)
        {
            counter++;
            if (*s1 == ',' || isspace(*s1))
            {
                while (*s1 == ',' || isspace(*s1))
                {
                    if (*s1 == ',')
                        commasCounter++;
                    s1++;
                }

                if (commasCounter > 1)
                    areOperandsLegal = yieldError(wrongOperationSyntaxExtraCommas);

                /*                 printf("counter -1:%d strlens2 - strlens1:%d\n", counter - 1, (int)(strlen(s2) - strlen(s1)));
                 */
                strcpy(second, s1);
                strncpy(first, s2, counter - 1);
                parsedOperands = True;
            }

            s1++;
        }
        while (*s1 && *s1 != '\0' && (!isspace(*s1) && *s1 != ','))
            s1++;
        second[strlen(second) - strlen(s1)] = '\0';
        if (strlen(first) > 0 && strlen(second) < 1)
        {
            strcpy(second, first);
            memset(first, 0, strlen(first));
            if (commasCounter > 0)
                areOperandsLegal = yieldError(illegalApearenceOfCommaAfterLastParameter);
        }

        commasCounter = 0;
        counter = 0;
        while (*s1 && *s1 != '\0')
        {
            if (isprint(*s1) && !isspace(*s1) && *s1 != ',')
                counter++;
            if (*s1 == ',')
                commasCounter++;

            s1++;
        }

        if (commasCounter > 0)
            areOperandsLegal = yieldError(illegalApearenceOfCommaAfterLastParameter);

        if (counter > 0)
            areOperandsLegal = yieldError(illegalApearenceOfCharactersInTheEndOfTheLine);

        /*         printf("second:%s first:%s s1:%s\n", second, first, s1); */

        /*
                if (*s1 == ',')
                {
                    yieldError(illegalApearenceOfCommaAfterLastParameter);
                    second[strlen(second) - strlen(s1) - 1] = '\0';
                }
                else if (!isspace(*s1) && isprint(*s1))
                    areOperandsLegal = yieldError(illegalApearenceOfCharactersInTheEndOfTheLine);
         */
        /*
                if (commasCounter > 1)
                    areOperandsLegal = yieldError(wrongOperationSyntaxExtraCommas); */

        else if (commasCounter < 1 && (strlen(first) && strlen(second)))
            areOperandsLegal = yieldError(wrongOperationSyntaxMissingCommas);
    }

    areOperandsLegal = parseOperands(first, second, p, active) && areOperandsLegal;
    if (areOperandsLegal)
    {
        int size = 2;
        if (active[0].immediate || active[1].immediate)
            size++;
        if ((active[0].direct || active[0].index) || (active[1].direct || active[1].index))
            size += 2;
        if (!p->funct && (!active[0].direct && !active[0].immediate && !active[0].index && !active[0].reg) && (!active[1].direct && !active[1].immediate && !active[1].index && !active[1].reg))
            size = 1;

        active[0].direct = active[0].immediate = active[0].index = active[0].reg = 0;
        active[1].direct = active[1].immediate = active[1].index = active[1].reg = 0;
        increaseInstructionCounter(size);
    }

    return areOperandsLegal ? lineParsedSuccessfully : Err;
}
Bool parseOperands(char *src, char *des, Operation *op, AddrMethodsOptions active[2])
{

    if (!op->src.direct && !op->src.immediate && !op->src.index && !op->src.reg && !op->des.direct && !op->des.immediate && !op->des.index && !op->des.reg && !*src && !*des)
        return True;
    else if ((op->src.direct || op->src.immediate || op->src.reg || op->src.index) && (op->des.direct || op->des.immediate || op->des.reg || op->des.index))
    {

        if (!src)
            return yieldError(requiredSourceOperandIsMissin);
        if (!des)
            return yieldError(requiredDestinationOperandIsMissin);

        return validateOperandMatch(op->src, active, src, 0) && validateOperandMatch(op->des, active, des, 1);
    }
    else if (op->src.direct || op->src.immediate || op->src.reg || op->src.index)
    {
        if (!src)
            return yieldError(requiredSourceOperandIsMissin);
        return validateOperandMatch(op->src, active, src, 0);
    }
    else if (op->des.direct || op->des.immediate || op->des.reg || op->des.index)
    {
        if (!des)
            return yieldError(requiredDestinationOperandIsMissin);
        return validateOperandMatch(op->des, active, des, 1);
    }

    return True;
}
Bool validateOperandMatch(AddrMethodsOptions allowedAddrs, AddrMethodsOptions active[2], char *operand, int type)
{
    Bool isImmediate = isValidImmediateParamter(operand);
    Bool isDirectIndex = !isImmediate && isValidIndexParameter(operand);
    Bool isReg = !isDirectIndex && !isImmediate && isRegistery(operand);
    Bool isDirect = !isReg && !isDirectIndex && !isImmediate && verifyLabelNaming(operand);

    if (!isReg && !isImmediate && !isDirect && !isDirectIndex)
        return type == 1 ? yieldError(desOperandTypeIsNotAllowed) : yieldError(srcOperandTypeIsNotAllowed);

    else if (!allowedAddrs.reg && isReg)
        return yieldError(operandTypeDoNotMatch);
    else if (!allowedAddrs.immediate && isImmediate)
        return yieldError(operandTypeDoNotMatch);
    else if (!allowedAddrs.direct && isDirect)
        return yieldError(illegalOperand);
    else if (!allowedAddrs.index && isDirectIndex)
        return yieldError(operandTypeDoNotMatch);

    active[type].direct = isDirect;
    active[type].reg = isReg;
    active[type].immediate = isImmediate;
    active[type].index = isDirectIndex;

    return True;
}

ParseState handleInstruction(int type, char *firstToken, char *nextTokens, char *line)
{

    if (isInstruction(firstToken))
    {

        if (type == _TYPE_DATA)
        {
            return countAndVerifyDataArguments(line) ? lineParsedSuccessfully : Err;
        }
        else if (type == _TYPE_STRING)
            return countAndVerifyStringArguments(line) ? lineParsedSuccessfully : Err;

        if (type == _TYPE_ENTRY || type == _TYPE_EXTERNAL)
        {
            if (nextTokens)
            {
                char *labelName = (char *)calloc(strlen(nextTokens), sizeof(char *));
                strcpy(labelName, nextTokens);
                nextTokens = strtok(NULL, " \t \n");
                if (nextTokens)
                {
                    yieldError(illegalApearenceOfCharactersInTheEndOfTheLine);
                    return Err;
                }
                else
                {
                    if (type == _TYPE_ENTRY)
                        return addSymbol(labelName, 0, 0, 0, 1, 0) ? lineParsedSuccessfully : Err;
                    if (type == _TYPE_EXTERNAL)
                        return addSymbol(labelName, 0, 0, 0, 0, 1) ? lineParsedSuccessfully : Err;
                }

                free(labelName);
            }
            else
            {
                yieldError(emptyDeclaretionOfEntryOrExternalVariables);
                return Err;
            }
        }
    }
    else if (isLabelDeclaration(firstToken))
    {
        int dataCounter = getDC();
        Bool isLabelNameAvailable;
        firstToken[strlen(firstToken) - 1] = '\0';
        isLabelNameAvailable = !isLabelNameAlreadyTaken(firstToken, Symbol);
        if (!isLabelNameAvailable)
            yieldError(illegalSymbolNameAlreadyInUse);

        if (((type == _TYPE_DATA && countAndVerifyDataArguments(line)) || (type == _TYPE_STRING && countAndVerifyStringArguments(line))) && isLabelNameAvailable)
        {

            return addSymbol(firstToken, dataCounter, 0, 1, 0, 0) ? lineParsedSuccessfully : Err;
        }
        else
            return Err;
    }
    else
        yieldError(undefinedOperation);

    return Err;
}
ParseState handleLabel(char *labelName, char *nextToken, char *line)
{
    if (!labelName || !nextToken || !line)
        return Err;
    if (isInstruction(nextToken))
    {
        int instruction = getInstructionType(nextToken);
        if (instruction == _TYPE_ENTRY || instruction == _TYPE_EXTERNAL)
        {
            char *next = strtok(NULL, " \t \n");
            if (next)
                return handleInstruction(instruction, nextToken, next, line);
            else
                return yieldWarning(emptyLabelDecleration);
        }
        else

            return handleInstruction(instruction, labelName, nextToken, line);
    }

    else if (isOperation(nextToken))
    {
        int icAddr = getIC();
        char args[MAX_LINE_LEN] = {0};
        int offset = (int)(strlen(labelName) + strlen(nextToken) + 1);
        strcpy(args, &line[offset]);
        if (handleOperation(nextToken, args))
            return addSymbol(labelName, icAddr, 1, 0, 0, 0) ? lineParsedSuccessfully : Err;
        else
            return Err;
    }

    else
        yieldError(illegalLabelUseExpectedOperationOrInstruction);

    return Err;
}
