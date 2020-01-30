# One-Time-Pads
Five small programs that encrypt and decrypt information using a one-time pad-like system. A Client sends a message, key, and an empty file to a server which encodes the message using the key. Another client will send the encoded message to a server to decode the message. 


To compile run compileall 

To run: 
1. Add a message you wish to encode to a file
2. Generate key with keygen using the same length as file above:
  ./keygen [key_leng] > [file_name]

3. Run servers in background
  Encoding Server: otp_enc_d [listening_port] (EX: $ otp_enc_d 57171 &) 
  Decoding Server: otp_dec_d [listening_port] ($ otp_dec_d 57171 &)    
 
 4.Encode and decode the message with the clients
  Encode Message Cliet: $ otp_enc [message_file] [key_file] [listening_port_of_enc] > [file to put code in]
  Decode Message Client: $ otp_dec [encode_message_file] [key_file] [listening_port_of_dec] > [file to put decoded message in]
  
  
