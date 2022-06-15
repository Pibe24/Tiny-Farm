#! /usr/bin/env python3

import sys,socket,threading

HOST="127.0.0.1"
PORT=57689


def gestisci_connessione(conn,addr,s):
  # in questo caso potrei usare direttamente conn
  # e l'uso di with serve solo a garantire che
  # conn venga chiusa all'uscita del blocco
  # ma in generale with esegue le necessarie
  # inzializzazioni e il clean-up finale
  with conn:  
      #print(f"Contattato da {addr}")
    # attendo due stringhe
    while True:

      #vado a leggere due interi
      
      #leggo il nome del file come stringa
      data = recv_all(conn,256)
      assert len(data)==256
      nome= data.decode("utf-8")

      if contains(nome,'fine'):
        s.shutdown(socket.SHUT_RDWR)
        break

      #controllo sulla terminazione
      if contains(nome,'No file'):
        break
      
      #leggo la somma come stringa
      data1=recv_all(conn,256)
      assert len(data1)==256
      somma=data1.decode("utf-8")      
      #vado a stampare il tutto 
      print(somma+" "+nome)
      
    


def recv_all(conn,n):
  chunks = b''
  bytes_recd = 0
  while bytes_recd < n:
    chunk = conn.recv(min(n - bytes_recd, 1024))
    if len(chunk) == 0:
      raise RuntimeError("socket connection broken")
    chunks += chunk
    bytes_recd = bytes_recd + len(chunk)
  return chunks


#funzione che mi dice se la sottostringa è contenuta o meno
def contains(name,string):
  a=name.find(string)
  if a==-1:
    return False
  else:
    return True

    

def main(host=HOST,port=PORT):

  with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((host,port))
    s.listen()
    #try per andare a catturare l'errore della socket shutdown
    try:
      while True:
        conn,addr=s.accept()
        #appena accetto la connessione lancio un thread con cui la vado a gestire
        t=threading.Thread(target=gestisci_connessione,args=(conn,addr,s))
        t.start()
    
    except socket.error:
      s.close()

    
if __name__ == '__main__':
  main()


    

