#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define VAULT_FILENAME "vault-notes.txt"
#define ALPHABET_SIZE 26



const char *rotorWiring[] =
{
    "EKMFLGDQVZNTOWYHXUSPAIBRCJ", // Rotor I
    "AJDKSIRUXBLHWTMCQGZNPYFVOE", // Rotor II
    "BDFHJLCPRTXVZNYEIWGAKMUSQO"  // Rotor III
};
const char rotorNotch[] = {'Q', 'E', 'V'};
const char *reflectorB = "YRUHQSLDPXNGOKMIEBFZCWVJAT";



void typewriter(const char *text, int delay);
void *encryptNote(char *usrMessage, int rotorPositions[3]);
void decryptNote();
void setupPlugboard(char plugboard[26]);
void setRotorPositions();
void stepRotors(int positions[]);
void saveToVault(const char *msgLabel, const char *encryptedMessage, const int rotorPositions[3]);
void deleteNote();
void cleanInput();

char encryptChar(char c, const char *rotors[], const int positions[], const char *reflector, const char plugboard[26]);
char plugboardSwap(char c, const char plugboard[26]);
char rotorForward(char c, const char *wiring, int offset);
char rotorReverse(char c, const char *wiring, int offset);
char reflect(char c, const char *reflector);




/*------------------------------------------------------
|   Prints banner and menu
+-----------------------------------------------------*/ 
void printMenu()
{
    system("cls");

     const char *banner =
        "=====================================================================\n"
        "=       ######## ##    ##       ##    ##      ##     ##             =\n"
        "=       ##       ###   ##       ###   ##      ##     ##             =\n"
        "=       ##       ####  ##       ####  ##      ##     ##             =\n"
        "=       ######   ## ## ##       ## ## ##      ##     ##             =\n"
        "=       ##       ##  ####       ##  ####       ##   ##              =\n"
        "=       ##       ##   ###  ###  ##   ###  ###   ## ##   ###         =\n"
        "=       ######## ##    ##  ###  ##    ##  ###    ###    ###         =\n"
        "=                                                                   =\n"
        "=              ENIGMA NOTE VAULT - ACCESS TERMINAL                  =\n"
        "=            CLASSIFIED - AUTHORIZED PERSONNEL ONLY                 =\n"
        "=====================================================================\n\n";
        typewriter(banner, 10);

        const char *menu = 
            "[1] CREATE ENCRYPTED NOTE\n"
            "[2] DECRYPT NOTE\n"
            "[3] VIEW ENCRYPTION LOG\n"
            "[4] DELETE NOTE\n"
            "[5] EXIT\n";

        typewriter(menu,50);
        Sleep(1000);
        typewriter("READY...\n\n", 50);
        Beep(1000,500);
        typewriter("COMMAND: ", 50);
}



/*---------------------------------------------------------
|   Prints to screen 1 char at a time with a given delay
+------------------------------------------------------- */
void typewriter(const char *text, int delay)
{
    for (int i = 0; text[i] != '\0'; i++)
    {
        putchar(text[i]);
        fflush(stdout);        // force immediate output
        Sleep(delay);          // delay in microseconds
    }
}



/*---------------------------------------------------------
|   Checks if the .txt file exists, if not it makes one
+------------------------------------------------------- */
void checkFile()
{
    FILE *file = fopen(VAULT_FILENAME, "r");

    if(file == NULL)
    {
        // If the file doesn't exist, make one
        file = fopen(VAULT_FILENAME, "w");
        
        if(file == NULL)
        {
            // If the file creation fails print error msg and quit
            printf("\nERROR! CANNOT CREATE %s\n", VAULT_FILENAME);
            exit(EXIT_FAILURE);
        }
        else
        {
            printf("\n>> VAULT FILE CREATED: %s", VAULT_FILENAME);
        }
    }
    else
    {
        // File already exists
        fclose(file);
    }
}



/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
|   Cleans STDIN to remove the possibility of '\n' skipping
|   input prompts
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void cleanInput()
{
    int ch = 0;
    while((ch = getchar()) != '\n' && ch != EOF); // flush any leftovers
}



/*---------------------------------------------------------
|   Asks for a string to encrypt
+------------------------------------------------------- */
void createNote()
{
    system("cls");

    const char *newNoteBanner =
        "======================================\n"
        "=         NEW ENCRYPTED NOTE         =\n"
        "======================================\n\n";
    typewriter(newNoteBanner, 50);
    Beep(1000,500);
    
    typewriter("LABEL: ", 50);
    char msgLabel[31];
    fgets(msgLabel, sizeof(msgLabel), stdin);
    msgLabel[strcspn(msgLabel, "\n")] = '\0';

    typewriter("MESSAGE: ", 50);
    char usrMessage[31];
    fgets(usrMessage, sizeof(usrMessage), stdin);
    usrMessage[strcspn(usrMessage, "\n")] = '\0';

    int rotorPositions[3];
    setRotorPositions(rotorPositions);

    char *encrypted = encryptNote(usrMessage, rotorPositions);
    saveToVault(msgLabel, encrypted, rotorPositions);

    free(encrypted);
}



/*---------------------------------------------------------
|   Sets up Enigma's plugboard swap combinations
|   (they HAVE TO be symmetrical)
+------------------------------------------------------- */
void setupPlugboard(char plugboard[26]) {
    for (int i = 0; i < 26; i++) {
        plugboard[i] = 'A' + i;  // No swap(identity)
    }

    // Symmetrical swaps
    plugboard['A' - 'A'] = 'M';
    plugboard['M' - 'A'] = 'A';

    plugboard['F' - 'A'] = 'T';
    plugboard['T' - 'A'] = 'F';

    plugboard['G' - 'A'] = 'Q';
    plugboard['Q' - 'A'] = 'G';

    plugboard['H' - 'A'] = 'L';
    plugboard['L' - 'A'] = 'H';

    plugboard['X' - 'A'] = 'Z';
    plugboard['Z' - 'A'] = 'X';
}



/*---------------------------------------------------------
|   To emulate the real Enigma, this function prompts
|   the user to give a starting position for each
|   rotor that will be used
+------------------------------------------------------- */
void setRotorPositions(int positions[3])
{
    system("cls");

    const char *rotorConfigBanner =
        "======================================\n"
        "=         SET ROTOR POSITIONS        =\n"
        "======================================\n\n";
    typewriter(rotorConfigBanner, 50);
    Beep(1000, 500);

    typewriter("ENTER STARTING ROTOR POSITIONS (3 LETTERS A-Z, e.g. A C Q): ", 50);

    char input[10];
    fgets(input, sizeof(input), stdin);

    // Transform input into tokens and convert them to rotor offsets
    char *token = strtok(input, " ");
    int index = 0;

    while(token != NULL && index < 3)
    {
        char ch = toupper(token[0]);
        if(ch >= 'A' && ch <= 'Z')
        {
            positions[index] = ch - 'A';
        } else
        {
            positions[index] = 0;  // Default fallback
        }
        index++;
        token = strtok(NULL, " ");
    }

    // If user provided fewer than 3 letters, fill the rest with 0
    while(index < 3)
    {
        positions[index++] = 0;
    }

    typewriter("\n>> ROTORS SET. READY TO ENCRYPT/DECRYPT.\n", 50);
    Sleep(2000);
}



/*---------------------------------------------------------
|   TO DO: WRITE COMMENT
+------------------------------------------------------- */
char plugboardSwap(char c, const char plugboard[26])
{
    if (c >= 'A' && c <= 'Z')
    {
        return plugboard[c - 'A'];
    }

    // Just in case it's not a capital letter
    return c;
}



/*---------------------------------------------------------
|   Will encrypt one char at a time by making it go 
|   through plugboard, rotors, reflectors and back
+------------------------------------------------------- */
char encryptChar(char c, const char *rotors[], const int positions[], const char *reflector, const char plugboard[26])
{
    // Step 1: Plugboard in
    c = plugboardSwap(c, plugboard);

    // Step 2: Forward through rotors (right to left)
    c = rotorForward(c, rotors[2], positions[2]);  // Rotor III
    c = rotorForward(c, rotors[1], positions[1]);  // Rotor II
    c = rotorForward(c, rotors[0], positions[0]);  // Rotor I

    // Step 3: Reflect
    c = reflect(c, reflector);

    // Step 4: Backwards through rotors (left to right), using reverse mapping
    c = rotorReverse(c, rotors[0], positions[0]);  // Rotor I
    c = rotorReverse(c, rotors[1], positions[1]);  // Rotor II
    c = rotorReverse(c, rotors[2], positions[2]);  // Rotor III

    // Step 5: Plugboard out
    c = plugboardSwap(c, plugboard);

    return c;
}



/*---------------------------------------------------------
|   Uses smaller helper functions to encrypt the
|   original user message
+------------------------------------------------------- */
void *encryptNote(char *usrMsg, int rotorPositions[3])
{
    // Adjust size if you need longer notes
    char encrypted[100];
    // Initial rotor positions (can be customized)
    // int rotorPositions[3] = {0, 0, 0};

    const char *rotors[] = 
    {
        "EKMFLGDQVZNTOWYHXUSPAIBRCJ", // Rotor I
        "AJDKSIRUXBLHWTMCQGZNPYFVOE", // Rotor II
        "BDFHJLCPRTXVZNYEIWGAKMUSQO"  // Rotor III
    };

    const char *reflector = "YRUHQSLDPXNGOKMIEBFZCWVJAT";

    char plugboard[26];
    // Creates the plugboard
    setupPlugboard(plugboard);

    int localPos[3] =
    {
        rotorPositions[0],
        rotorPositions[1],
        rotorPositions[2]
    };

    for(int i = 0; usrMsg[i] != '\0'; i++)
    {
        if (isalpha(usrMsg[i]))
        {
            char upper = toupper(usrMsg[i]);
            encrypted[i] = encryptChar(upper, rotors, localPos, reflector, plugboard);
            stepRotors(localPos);
        }
        else
        {
            // Leaves punctuation intact
            encrypted[i] = usrMsg[i];
        }
    }
    // NULL terminating the result
    encrypted[strlen(usrMsg)] = '\0';

    char *result = malloc(strlen(encrypted) +1);
    strcpy(result, encrypted);
    return result;
}



/*---------------------------------------------------------
|   Sends a string provided by the user through the 
|   encrypt function thus if presented with an already
|   encrypted message, the function will show the
|   original message in plain text
+------------------------------------------------------- */
void decryptNote()
{
    system("cls");

    const char *decryptNoteBanner =
        "======================================\n"
        "=             DECRYPT NOTE           =\n"
        "======================================\n\n";
    typewriter(decryptNoteBanner, 50);
    Beep(1000,500);

    typewriter("ENTER NOTE TO DECRYPT: ", 50);
    char usrMessage[31];
    fgets(usrMessage, sizeof(usrMessage), stdin);
    usrMessage[strcspn(usrMessage, "\n")] = '\0';

    int rotorPositions[3];
    setRotorPositions(rotorPositions);

    char *decrypted = encryptNote(usrMessage, rotorPositions);


    typewriter("\n\n>> DECRYPTED NOTE: ", 50);
    printf("%s ", decrypted);
    printf("\n");

    free(decrypted);
}




/*---------------------------------------------------------
|   Send a single char through the rotors(Rx to Lx)
+------------------------------------------------------- */
char rotorForward(char c, const char *wiring, int offset)
{
    // Convert character to 0–25 index and apply offset
    int inputPos = (c - 'A' + offset) % ALPHABET_SIZE;

    // Map through rotor wiring
    char mappedChar = wiring[inputPos];

    // Convert back to letter and apply inverse offset
    int output = (mappedChar - 'A' - offset + ALPHABET_SIZE) % ALPHABET_SIZE;

    return 'A' + output;
}



/*---------------------------------------------------------
|   Send the swapped char backwards (Lx to Rx)
+------------------------------------------------------- */
char rotorReverse(char c, const char *wiring, int offset)
{
    // Shift input by offset
    int shifted = (c - 'A' + offset) % ALPHABET_SIZE;

    // Search for matching output in the wiring
    for (int i = 0; i < ALPHABET_SIZE; i++)
    {
        int wired = (wiring[i] - 'A' + ALPHABET_SIZE) % ALPHABET_SIZE;
        if (wired == shifted)
        {
            // Apply inverse offset to return back into standard space
            return 'A' + (i - offset + ALPHABET_SIZE) % ALPHABET_SIZE;
        }
    }

    // Fallback safeguard
    return c;
}



/*---------------------------------------------------------
|   HOW DO I EVEN EXPLAIN THIS?! :/
+------------------------------------------------------- */
void stepRotors(int positions[])
{
    int rotor1Pos = positions[0]; // Left (Rotor I)
    int rotor2Pos = positions[1]; // Middle (Rotor II)
    int rotor3Pos = positions[2]; // Right (Rotor III)

    // Check if Rotor II is at its notch to cause Rotor I to step
    int middleAtNotch = (rotorWiring[1][rotor2Pos] == rotorNotch[1]);

    // Check if Rotor III is at its notch to step Rotor II
    int rightAtNotch = (rotorWiring[2][rotor3Pos] == rotorNotch[2]);

    // Rotor I (leftmost) steps if Rotor II is at its notch (double-stepping)
    if (middleAtNotch)
    {
        positions[0] = (rotor1Pos + 1) % ALPHABET_SIZE;
    }

    // Rotor II (middle) steps if Rotor III is at its notch OR it double-steps
    if (rightAtNotch || middleAtNotch)
    {
        positions[1] = (rotor2Pos + 1) % ALPHABET_SIZE;
    }

    // Rotor III always steps
    positions[2] = (rotor3Pos + 1) % ALPHABET_SIZE;
}


/*--------------------------------------------
|   TO DO: WRITE COMMENT
+-------------------------------------------*/
char reflect(char c, const char *reflector)
{
    return reflector[c - 'A'];
}



/*---------------------------------------------------
|   Saves label and encrypted message to file
+---------------------------------------------------*/
void saveToVault(const char *msgLabel, const char *encryptedMessage, const int rotorPositions[3])
{
    FILE *file = fopen(VAULT_FILENAME, "a");  // 'a' for append

    if (file == NULL)
    {
        printf("ERROR: COULD NOT OPEN FILE...\n");
        printf("\nPRESS ENTER TO RETURN TO MAIN MENU...");
        getchar();
        return;
    }

    // Write the label and message to file
    fprintf(file, "%s|%s|%d|%d|%d\n", msgLabel, encryptedMessage,
            rotorPositions[0], rotorPositions[1], rotorPositions[2]);
    fclose(file);
    printf(">> ENCRYPTED MESSAGE SAVED TO VAULT\n");
    //printf("\nPRESS ENTER TO RETURN TO MAIN MENU...");
    //getchar();
}



/*-----------------------------------------------------
|   Allows the user to view the list of messages
+----------------------------------------------------*/
void viewVault()
{
    system("cls");

    printf("ACCESSING ENCRYPTED FILE SYSTEM...\n\n");
    printf("[");
    for (int i = 0; i < 30; i++)
    {
        printf("|");
        Sleep(30);
    }
    printf("]\n");
    Beep(1000, 80);
    Beep(1400, 80);
    Sleep(500);
    system("cls");

    const char *viewNoteBanner =
        "======================================\n"
        "=         VIEW ENCRYPTION LOG        =\n"
        "======================================\n\n";
    typewriter(viewNoteBanner, 50);
    Beep(1000,500);

    FILE *file = fopen(VAULT_FILENAME, "r");
    if (file == NULL)
    {
        printf(">> NO VAULT FILE FOUND...\n");
        getchar();
        return;
    }

    char line[256];
    int count = 1;
    while (fgets(line, sizeof(line), file))
    {
        // Skip empty lines
        if (line[0] == '\n' || line[0] == '\0')
        {
            continue;
        }

        // Separate label and encrypted message
        char *label = strtok(line, "|");
        char *message = strtok(NULL, "|");
        char *r1 = strtok(NULL, "|");
        char *r2 = strtok(NULL, "|");
        char *r3 = strtok(NULL, "\n");

        if (label && message && r1 && r2 && r3)
        {
            printf("-> [%03d] Label   : %s\n", count, label);
            printf("         Cipher  : %s\n\n", message);
            printf("         Rotors  : [%s %s %s]\n\n", r1, r2, r3);
            count++;
            Sleep(500);
        }
    }

    fclose(file);

    //printf("\nPRESS ENTER TO RETURN TO MAIN MENU...");
    //getchar();
    
}



/*-----------------------------------------------------
|   Allows the user to delete a message by using its
|   label as ID
+----------------------------------------------------*/
void deleteNote()
{
    system("cls");

    const char *deleteNoteBanner =
        "======================================\n"
        "=        DELETE NOTE BY NUMBER       =\n"
        "======================================\n\n";
    typewriter(deleteNoteBanner, 50);
    Beep(1000,500);

    FILE *file = fopen(VAULT_FILENAME, "r");
    FILE *temp = fopen("temp.txt", "w");

    if (file == NULL || temp == NULL)
    {
        printf("ERROR: COULD NOT OPEN FILE...\n");
        getchar();
        return;
    }

    char line[256];
    int currentIndex = 1;
    int targetIndex = 0;

    typewriter("ENTER NUMBER [N]: ", 50);
    scanf(" %d", &targetIndex);

    cleanInput();
    typewriter("\nCONFIRMING CHOICE (Y = Yes / N = No): ", 50);
    char usrChoice = '\0';
    scanf(" %c", &usrChoice);
    usrChoice = toupper(usrChoice);

    switch(usrChoice)
    {
        case 'Y':
            while (fgets(line, sizeof(line), file))
            {
                if (currentIndex != targetIndex)
                {
                    fputs(line, temp);
                }
                currentIndex++;
            }

            fclose(file);
            fclose(temp);

            remove(VAULT_FILENAME);
            rename("temp.txt", VAULT_FILENAME);
    
            typewriter("\n>> NOTE ", 50);
            printf("[%d]", targetIndex);
            typewriter("HAS BEEN DELETED\n", 50);
            getchar();
            break;
        case 'N':
            typewriter("HALTING DELETION...", 50);
            getchar();
            break;
        default:
            typewriter("UNRECOGNIZED INPUT...", 50);
            getchar();
            break;
    }

}



/*--------------------------
|   MAIN FUNCTION
+-------------------------*/
int main()
{
    int usrChoice = 0;

    system("cls");
    system("color 0a");

    Beep(1000,500);
    const char *message = "ACCESS GRANTED. WELCOME TO ENIGMA VAULT...\n";
    typewriter(message, 50); // 50ms delay per character

    while(1)
    {
        printMenu();

        scanf(" %d", &usrChoice);

        switch(usrChoice)
        {
            case 1:
                typewriter(">> CREATING NEW ENCRYPTED NOTE...\n", 50);
                Sleep(1500);
                checkFile();
                cleanInput();
                createNote();
                break;
            case 2:
                typewriter(">> DECRYPTING NOTE...\n", 50);
                Sleep(1500);
                cleanInput();
                checkFile();
                decryptNote();
                break;
            case 3:
                typewriter(">> VIEWING NOTES...\n", 50);
                Sleep(1500);
                cleanInput();
                checkFile();
                viewVault();
                break;
            case 4:
                typewriter(">> DELETING NOTE...\n", 50);
                Sleep(1500);
                cleanInput();
                checkFile();
                deleteNote();
                break;
            case 5:
                exit(0);
            default:
                printf("UNRECOGNIZED INPUT...RETRY...\n");
                break;
        }

        printf("PRESS ENTER TO CONTINUE...\n");
        getchar(); // Wait for user

    }

    return 0;
}