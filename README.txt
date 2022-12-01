//Project for CSC-400

Notes from Krupp 11/4:
 	- Client and server can be on one file
   	- Client will be sending the codes
   	- Make sure two threads can't grab the same file
	- Save command doesn't actually create the file, we just send the command
	- Netcat can simulate sending/receiving commands, hardcode responses from readFile function
	- Maybe make a struct for saveFile info?


//Pseudocode:
//test

main(){
    input = recieveUserInput(); // recieves user input
    operation, filename, contents = inputHandler(input); // cleans the input and separates it into its specific parts (operation, filename and/or contents)
    if (readNumThreads()){ // detects if there is a thread available and splits a thread to handle the operation
        if(operation == save){
            pthread_create(saveFile(), filename, contents);
        }
        else if(operation == read){
            pthread_create(readFile(), filename);
        }
        else if(operation == delete){
            pthread_create(deleteFile(), filename);
        }
        else{
            print("unknown command");
        }
    }
    else{
        print("no threads available");
    }
}

saveFile(filename, contents){
    file.create(filename); //creates a file with the filename
    filename.insert(contents); //inserts the contents into the file
    sendSave(fileServer, 'write ' + filename + ' ' + contents); //sends API command to the File Server with format 'write filename n:contents'
}

readFile(filename){
    fileCheck = loadMemory('load ' + filename); //sends API command to Memory Cache with format 'load filename'
    if(filecheck != '0:'){ //sees if the cache returned a valid result
        return filecheck;
    }
    else if(filecheck == '0:'){ //checks if cache returned an absent value
        if(loadFile('read ' + filename) != '0:'){ //sends API command to File Server to see if it is in there
            sendSave(memoryCache, 'store ' + filename + ' ' + contents); //sends API command to memory cache to save the file
            return filecheck;
        }
        else{
            return 'File does not exist';
        }
    }
    else{
        return 'Invalid API call';
    }
}

deleteFile(filename){
    sendDelete(memoryCache, 'rm ' + filename); //sends API command to the Memory Cache with format 'rm filename'
    sendDelete(fileSystem, 'delete ' + filename); //sends API command to the File Server with format 'delete filename'
}

11/29 notes:

server {
	main {
		get packets from network
		splits up message and
	}

	client {


		if op = save {
			clientSend(message, fileport, NULL)
		}
		else if op = read {

		}
		else{
			clientSend(message, cacheport, NULL)
			clientSend(message, fileport, NULL)
		}

		clientSend (char * message, int port, &response){

		}
	}
}

