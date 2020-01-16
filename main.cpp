#include <iostream>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/utsname.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <sstream>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported

// Clearing the shell using escape sequences
#define clear() printf("\033[H\033[J")
using namespace std;
class cwushell {
public:
    struct Prompt {
        char *prompt;
    };

    void init_shell()
    {
        clear();
        char* username = getenv("USER");
        printf("\n\n\nUSER is: @%s", username);
        printf("\n");
        sleep(1);
        clear();
    }
    // Function to take input
    int takeInput(char* str, const char* input)
    {
        char* buf;
        buf = readline(input);
        if (strlen(buf) != 0) {
            add_history(buf);
            strcpy(str, buf);
            return 0;
        } else {
            return 1;
        }
    }
    void execArgs(char** parsed)
    {
        // Forking a child
        pid_t pid = fork();

        if (pid == -1) {
            printf("\nFailed forking child..");
            return;
        } else if (pid == 0) {
            if (execvp(parsed[0], parsed) < 0) {
                printf("\nCould not execute command..");
            }
            exit(0);
        } else {
            // waiting for child to terminate
            wait(NULL);
            return;
        }
    }
    void openHelp()
    {
        puts("\nList of Commands supported:"
             "\n>quit [n] -- terminates the shell with exic code n. Otherwise returns the value returned by the last command"
             "\n>change_prompt [new_prompt_name] -- changes the prompt name to [new_prompt_name]. If no new name is provided, returns the prompt to 'cwushell'"
             "\n>distro -switch -- prints different cpu related information depending on the switch"
             "\n     -v -- print the distro's version"
             "\n     -n -- print the distro's name"
             "\n     -c -- print the distro's codename"
             "\n>info -switch -- print different memory related info based on switch"
             "\n     -s -- print the current memory page size in KB"
             "\n     -n -- print the currently available page numbers"
             "\n     -p -- print the number of available processors"
             "\n>all other existing shell commands"
             "\n>improper space handling");

        return;
    }
    char * processData(char * buffer, char** parsed) {
        int i = 0;
        int choiceNum = 0;
        char * choice = parsed[1];
        if(strcmp(choice, "-v") == 0) {
            choiceNum = 3;
        } else if(strcmp(choice, "-n") == 0) {
            choiceNum = 1;
        } else if(strcmp(choice, "-c") == 0) {
            choiceNum = 4;
        } else {
            cout << "Invalid switch... try again" << endl;
        }
        char * pch;
        pch = strtok(buffer, " ");
        while(pch != NULL) {
            //if(i == choiceNum) {
                printf("%s\n",pch);
            //}
            pch = strtok(NULL, "=");
            i++;
        }
    }
    void distro(char** parsed) {
        if(parsed[1] != NULL) {
            FILE * file;
            file = fopen("/etc/lsb-release", "r");
            fseek(file, 0, SEEK_END);
            long lSize = ftell(file);
            rewind(file);

            char * buffer = (char*) malloc (sizeof(char)*lSize);
            if(buffer == NULL) {
                fputs("Memory error", stderr);
                exit(2);
            }
            size_t result = fread(buffer,1,lSize,file);
            if(result != lSize) {
                fputs("Reading error", stderr);
                exit(3);
            }

            processData(buffer, parsed);
            fclose(file);
            free(buffer);
        } else {
            cout << "No switch provided... Open help with command 'help' if you don't know which switch to use..." << endl;
        }

    }
    void info(char** parsed) {

    }
    void change_prompt(char** parsed, Prompt * thisPrompt) {
        if(parsed[1] != NULL) {
            char newPrompt[] = ">";
            strcat(parsed[1], newPrompt);
            thisPrompt->prompt = parsed[1];
            cout << "Prompt changed to " << parsed[1] << endl;
        } else {
            char newPrompt[] = "cwushell>";
            thisPrompt->prompt = newPrompt;
            cout << "Prompt reverted to cwushell>\n";
        }

    }
    void quit(char** parsed) {
        if(parsed[1] != NULL) {
            int exitCode = *parsed[1] - '0';
            cout << "Exiting with code: " << exitCode << endl;
            exit(exitCode);
        }else {
            cout << "Default exit..." << endl;
            exit(0);
        }
    }
    int ownCmdHandler(char** parsed, Prompt * thisPrompt)
    {
        int NoOfOwnCmds = 5, i, switchOwnArg = 0;
        const char* ListOfOwnCmds[NoOfOwnCmds];
        char* username;

        ListOfOwnCmds[0] = "quit";
        ListOfOwnCmds[1] = "change_prompt";
        ListOfOwnCmds[2] = "distro";
        ListOfOwnCmds[3] = "info";
        ListOfOwnCmds[4] = "help";

        for (i = 0; i < NoOfOwnCmds; i++) {
            if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
                switchOwnArg = i + 1;
                break;
            }
        }

        switch (switchOwnArg) {
            case 1:
                quit(parsed);
                return 1;
            case 2:
                change_prompt(parsed, thisPrompt);
                return 1;
            case 3:
                distro(parsed);
                return 1;
            case 4:
                //info(parsed);
                return 1;
            case 5:
                openHelp();
                return 1;
            default:
                break;
        }

        return 0;
    }
    void parseSpace(char* str, char** parsed)
    {
        int i;

        for (i = 0; i < MAXLIST; i++) {
            parsed[i] = strsep(&str, " ");

            if (parsed[i] == NULL)
                break;
            if (strlen(parsed[i]) == 0)
                i--;
        }
    }
    int processString(char* str, char** parsed, Prompt * thisPrompt)
    {
        parseSpace(str, parsed);
        if (ownCmdHandler(parsed, thisPrompt))
            return 0;
    }
    int main()
    {

        char inputString[MAXCOM], *parsedArgs[MAXLIST];
        char* parsedArgsPiped[MAXLIST];
        int execFlag = 0;
        Prompt thisPrompt;
        char input[] = "cwushell>";
        thisPrompt.prompt = input;
        init_shell();
        while (1) {
            // take input
            if (takeInput(inputString, thisPrompt.prompt))
                continue;
            // process
            execFlag = processString(inputString,
                                     parsedArgs, &thisPrompt);
            // execute
            if (execFlag == 1)
                execArgs(parsedArgs);


        }
        return 0;
    }
};

int main() {
    cwushell * shell;
    shell->main();
    return 0;
}