from tkinter import *
import serial

connexion = serial.Serial('COM3', 9600)
distanceActuel = 4
limit = IntVar()


def lecture():
    data = (connexion.readline()).decode()
    rep = data.split()

    if rep[0] == "IN":
        libelleInfo.configure(text="Dans la limite", fg='green')
        distanceActuel = rep[1]
    if rep[0] == "OUT":
        libelleInfo.configure(text="En dehors de la limite", fg='red')
        distanceActuel = rep[1]

    connexion.flush()


def clicked():
    connexion.write(bytes("#" + entrySet.get(), 'utf-8'))


window = Tk()
window.title('recording')
libelleSet = Label(window, text="Distance limit:")
libelleSet.grid(column=0, row=0)
entrySet = Entry(window, textvariable=limit)
entrySet.grid(column=0, row=1)
btnSet = Button(window, text="Envoyer", command=clicked)
btnSet.grid(column=0, row=2)
libelleDistance = Label(window, text='Distance actuelle: ' + str(distanceActuel) + ' cm')
libelleDistance.grid(column=0, row=3)
libelleInfo = Label(window, text='')
window.update()

connexion.close()
connexion.open()
try:
    while 1:
        if connexion.inWaiting() > 0:
            lecture()
            window.update()
except KeyboardInterrupt:
    connexion.close()
