#! /usr/bin/env python3

import sys,socket,threading,struct,time

HOST="127.0.0.1"
PORT=65432


def gestisci_connessione(conn,addr,a,s):
  # in questo caso potrei usare direttamente conn
  # e l'uso di with serve solo a garantire che
  # conn venga chiusa all'uscita del blocco
  # ma in generale with esegue le necessarie
  # inzializzazione e il clean-up finale
  with conn:  
      #print(f"Contattato da {addr}")
    # attendo due stringhe
    while True:
      #leggo il nome del file come stringa
      data = recv_all(conn,256)
      assert len(data)==256
      nome= data.decode(encoding="utf-8",errors="ignore")

      if not notcontains(nome,'fine'):
        a["terminazione"]=1;
        print("il messaggio di termine è arrivato")
        break

      #controllo sulla terminazione
      if not notcontains(nome,'No file'):
        break
      #print(a)
      #leggo la somma come stringa
      data1=recv_all(conn,256)
      assert len(data1)==256
      
      somma=data1.decode("utf-8")
      print(somma+" "+nome)
      
   # print(f"Finito con {addr}")
    


# farm z0.dat z1.dat z2.dat6355299 z3.dat-1906813

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



def notcontains(name,string):
  a=name.find(string)
  if a==-1:
    return True
  else:
    return False

    

def main(host=HOST,port=PORT):

  with socket.socket(socket.AF_INET,socket.SOCK_STREAM) as s:
    s.bind((host,port))
    s.listen()
    a={"terminazione":0}
    while True:
     # print("Mi metto in attesa di un client...")
      conn,addr=s.accept()
      #appena accetto la connessione lancio un thread con cui la vado a gestire
      t=threading.Thread(target=gestisci_connessione,args=(conn,addr,a,s))
      #if(a["terminazione"]<=1 and a["bool"]==False):
       # break
      #else:
      t.start()
      if a["terminazione"]==1:
        break
      
      
    print("messaggio di terminazione arrivato smetto connessione chiusa")
    s.shutdown(socket.SHUT_RDWR)

    
if __name__ == '__main__':
  main()


    

