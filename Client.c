/* Header files */
#include <stdio.h> 
#include <stdbool.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>

/* Defines */
#define PORT 54321    /* the default port client will be connecting to */
#define MAXDATASIZE 100 /* max number of bytes we can get at once */
#define CHAR_SIZE 20	/* Max Char size for password and username */
#define PLAY_GAME 1
#define LEADERBOARD 2
#define QUIT 3

/* Global Varaibles */



/* Structs */
struct Credentials{
	char userName[CHAR_SIZE];
	char passWord[CHAR_SIZE];
} credentials;

struct hostent;
struct sockaddr;


/* Function Prototypes */
void SetupSocket(int argc, char* argv[], int* portNumber, int* sockfd, struct hostent *he,\
	struct sockaddr_in *their_addr);

bool Logon(int sockfd);
void LogonScreen();
void GetCredentials();
void GetUserName();
void GetPassWord();
bool RequestServer(int sockfd);
void SendErrorCode(int errorCode);
void SendPrompt(int errorCode);

void MainMenu();
void MainMenuScreen();
void GetUserInput(int* choice);
bool CheckGamesPlayed(int choice, int sockfd);
bool SendChoiceToServer();

void RecvNumberFrom_Server(int sockfd, int* number);

void ShowLeaderBoard(int sockfd);
void PlayGame(int sockfd);

/* Funciton Implementations */

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SetupSocket ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SetupSocket(int argc, char* argv[], int* portNumber, int* sockfd, struct hostent *he,\
	struct sockaddr_in *their_addr){
	if(argc < 3){
		fprintf(stderr, "Error: Not Enough Inputs\n");
		exit(1);
	}
	int server_IP_address = atoi(argv[1]);
	*portNumber = atoi(argv[2]);

	if((he=gethostbyname(argv[1])) == NULL){
		herror("gethostbyname");
		exit(1);
	}

	if((*sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket");
		exit(1);
	}

	(*their_addr).sin_family = AF_INET;      /* host byte order */
	(*their_addr).sin_port = htons(*portNumber);    /* short, network byte order */
	(*their_addr).sin_addr = *((struct in_addr *)he->h_addr);
	bzero(((*their_addr).sin_zero), 8);     /* zero the rest of the struct */

	/* Connect Socket */
	if (connect(*sockfd, (struct sockaddr *)their_addr, \
		sizeof(struct sockaddr)) == -1) {
		perror("connect");
	exit(1);
}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ Logon ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
bool Logon(int sockfd){
	bool authentic = false;

	LogonScreen();
	GetCredentials();
	authentic = RequestServer(sockfd);
	if(!authentic){
		int errorCode = 0;
		SendPrompt(errorCode);
	}
	return true;	
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ LogonScreen ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void LogonScreen(){
	printf("\n=============================================\n\n\n");
	printf("Welcome to the Online Hangman Gaming System\n\n\n");
	printf("=============================================\n\n\n\n");
	printf("You are required to logon with your registered Username and PassWord\n\n");
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GetCredentials ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void GetCredentials(){
	GetUserName();
	GetPassWord();
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GetUserName ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void GetUserName(){
	char buffer[20];
	printf("Please enter your username-- >");
	fgets(buffer, sizeof(credentials.userName), stdin);
	sscanf(buffer, "%s", credentials.userName);		/* use sscanf() to remove newline char */
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GetPassword~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void GetPassWord(){
	char buffer[20];
	printf("Please enter your password-- >");
	fgets(buffer, sizeof(credentials.passWord), stdin);
	sscanf(buffer, "%s", credentials.passWord);		/* use sscanf() to remove newline char */
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RequestServer  ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
bool RequestServer(int sockfd){
	/* Send username*/
	if(send(sockfd, &(credentials.userName), 9*sizeof(char), 0) == -1){
		perror("send: ");		
	}

	/* Send password*/
	if(send(sockfd, &(credentials.passWord), 9*sizeof(char), 0) == -1){
		perror("send: ");
	}

	/* Revieve Confirmation from Server */
	char buf[20]; int numbytes;

	if ((numbytes=recv(sockfd, buf, 18*sizeof(char), 0)) == -1) {
		perror("recv:");
		exit(1);
	}

	if(strcmp(buf, "N") == 0){
		return false;
	}else return true;	
}



/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SendPrompt~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void SendPrompt(int errorCode){
	if(errorCode == 0){
		printf("\nYou entered either an incorrect username or password - disconnecting\n");
		exit(EXIT_FAILURE);
	}
	
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ MainMenuScreen~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void MainMenuScreen(){
	printf("\n\n\nWelcome to the Hangman Gaming System\n\n\n");
	printf("Please enter a selection\n");
	printf("<1> Play Hangman\n");
	printf("<2> Show Leaderboard\n");
	printf("<3> Quit\n");
	printf("\nSelection option 1-3 ->");
	fflush(stdout);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ GetUserInput ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void GetUserInput(int* choice){
	char buff_buff[2]; int numVariables = 0; int c;

	/* Flush stdin */
	//while((c = getchar()) != '\n' && c != EOF){printf("%d\n", c);};
	//*choice = getchar();
	numVariables = scanf("%d", choice);
	//sscanf(buff_buff, "%d", choice);	


	/* Need to have this check in of will instantiate choice to 0 */


	//while(*choice == 0){
	//	fgets(buff_buff, sizeof(char), stdin);
	//	numVariables = sscanf(buff_buff, "%d", choice);		/* use sscanf() to remove newline char */

	//printf("var1 = %d var2 = %d\n",numVariables, *choice);
	//}
	//while(1);

}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ SendChoiceToServer ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
bool SendChoiceToServer(int choice, int sockfd){
	int result; int length;

	/* Send choice */
	uint16_t network_byte_order_short = htons(choice);

	if((length = send(sockfd, &network_byte_order_short, sizeof(uint16_t), 0)) == -1){
		perror("send: ");		
	}

	//printf("\nSendChoiceToServer: just sent choice : %d of mesg length %d\n", choice, length);

	/* Recieve Confirmation from Server */
	RecvNumberFrom_Server(sockfd, &result);

	//printf("SendChoiceToServer: result: %d\n", result);

	if(result == 1){
		return true;
	}else return false;	
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ RecvNumberFrom_Server ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void RecvNumberFrom_Server(int sockfd, int* number){
	uint16_t buffer = 0;
	int numbytes;

	/* Recieve from Server if there is any information */
	if ((numbytes=recv(sockfd, &buffer, sizeof(uint16_t), 0)) == -1) {
		perror("recv:");
		exit(1);
	}
	*number = ntohs(buffer);
	//printf("RecvNumberFrom_Server: The size of the last msg was: %d\n", numbytes);
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ShowLeaderBoard ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
STILL TODO: 
*/
void ShowLeaderBoard(int sockfd){
	uint16_t buffer = 0;
	int result, numbytes, number_of_players;

	//printf("Im at the Leaderboard\n");


	/* Recieve from Server if there is any information */
	RecvNumberFrom_Server(sockfd, &result);

	//printf("Display Confirmation %d\n", result);

	//printf("hello\n");

	if(result == 0){
		printf("\n==============================================================================\n\n");
		printf("There is no information currently stored in the Leader Board. Try again latter\n\n");
		printf("==============================================================================\n");
	}else{

		// /* Lets synch up */
		// int toSend = 1;
		// uint16_t network_byte_order_short = htons(toSend);
		// if(send(sockfd, &network_byte_order_short, sizeof(uint16_t), 0) == -1){
		// 	perror("send: ");		
		// }


		/* Recieve how many players  - will have for loop that repeats next steps*/
		RecvNumberFrom_Server(sockfd, &number_of_players);
		printf("\n==============================================================================\n\n");
		//printf("There is info in the Leaderboard, coming shortly\n");

		//printf("number of players = -%d-\n", result);
		for(int ii = 0; ii < number_of_players; ii++){


		/*Recieve thier name */
			char buf[11];

			if ((numbytes=recv(sockfd, buf, 11*sizeof(char), 0)) == -1) {
				perror("recv:");
				exit(1);
			}

			printf("\nPlayer  - %s\n", buf);

		/* Recieve thier number of games won */
			RecvNumberFrom_Server(sockfd, &result);
			printf("Number of games won  - %d\n", result);
			fflush(stdout);

		/*Recieve their number of games played */
			RecvNumberFrom_Server(sockfd, &result);
			printf("Number of games played  - %d\n\n", result);
			printf("==============================================================================\n");

		}

	}
}


/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ PlayGame ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void PlayGame(int sockfd){
	int havewon = 0;
	char* buffer;
	int guess_left, numbytes;
	char guess[1];
	char alreadyGuessed[30]; //wont ever be more than 26 guesses
	char wordfromthe_Server[30]; // the word sent back with correct guesses added in

	RecvNumberFrom_Server(sockfd, &guess_left);

	// WHILE LOOP
	while(guess_left > 0){

	// Recieve from Server, guesses, Print guessed letters
		if ((numbytes=recv(sockfd, alreadyGuessed, 30*sizeof(char), 0)) == -1) {
			perror("recv:");
			exit(1);
		}
		printf("\n\nGuessed letters: %s\n", alreadyGuessed);

	// Recieve how many guesses left
		RecvNumberFrom_Server(sockfd, &guess_left);
		printf("\nNumber of guesses left: %d\n", guess_left);

	// Recive from Server the word, Print word
		if ((numbytes=recv(sockfd, wordfromthe_Server, 30*sizeof(char), 0)) == -1) {
			perror("recv:");
			exit(1);
		}
		printf("\nWord: %s\n", wordfromthe_Server);

	// Printt 'Enter your guess'
		printf("\nEnter your guess - ");fflush(stdout);
		scanf("%s", guess);	


	//send guess to server
		if(send(sockfd, guess, sizeof(char), 0) == -1){
			perror("send: ");	
		}

		// needed so client can exit
		RecvNumberFrom_Server(sockfd, &guess_left);
		printf("\n\n----------------------------------------------------------------------------\n");


	//recieve yes or no from sever to see if have won
		RecvNumberFrom_Server(sockfd, &havewon);
		if(havewon){
			break;
		}
	}// END WHILE LOOP	



	/* End of Game info */
		// Recieve from Server, guesses, Print guessed letters
	if ((numbytes=recv(sockfd, alreadyGuessed, 30*sizeof(char), 0)) == -1) {
		perror("recv:");
		exit(1);
	}
	printf("\n\nGuessed letters: %s\n", alreadyGuessed);

		// Recieve how many guesses left
	RecvNumberFrom_Server(sockfd, &guess_left);
	printf("\nNumber of guesses left: %d\n", guess_left);

		// Recive from Server the word, Print word
	if ((numbytes=recv(sockfd, wordfromthe_Server, 30*sizeof(char), 0)) == -1) {
		perror("recv:");
		exit(1);
	}
	printf("\nWord: %s\n", wordfromthe_Server);

	// Display strings depending on the game result
	if(havewon){
		printf("\nGame over\n\n");
		printf("\nWell done %s! You won this round of Hangman!\n", credentials.userName);
	}else{
		printf("\nGame over\n\n");
		printf("\nBad Luck %s! You have run out of guesses. The Hangman got you!\n", credentials.userName);
	}

}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
/* ~~~~~~~ Main ~~~~~~~~~~*/
int main(int argc, char* argv []){
	// socket variables
	int sockfd, numbytes, portNumber = PORT; int choice = 0; 
	char buf[MAXDATASIZE];
	struct hostent *he;
	struct sockaddr_in their_addr; /* connector's address information */
	bool inputValid = false;

	/* Set up socket */
	SetupSocket(argc,argv, &portNumber, &sockfd, he, &their_addr);

	/* Logon to the Server */
	Logon(sockfd);


	while(choice != 3){
		while(!inputValid){
			MainMenuScreen();
			GetUserInput(&choice); /* Updates value of choice */
			inputValid = SendChoiceToServer(choice, sockfd); /*Sends choice to Server and gets back if valid */			
		}
		inputValid = false; /* Reset this to false */

		if(choice == QUIT){
			close(sockfd);    /* NEED to find out if only one side does this */
			exit(1);

		}else if(choice == LEADERBOARD){
			ShowLeaderBoard(sockfd);

		}else{ /* PLAY_GAME */
			PlayGame(sockfd);
		} choice = 0;
	} 
}


/* Unused Funcitons */
