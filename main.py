from tkinter import *
import serial

try:
    connexion = serial.Serial('COM3', 9600)
    print("connexion réussit")
except Exception as error:
    print(error)
    exit()
distanceActuel = 4
limit = 0
affichage = ""


def lecture():
    data = connexion.readline().decode()
    print(data)
    rep = data.split(" ")
    data = data[:-1]

    if rep[0] == "IN":
        libelleInfo.configure(text="Dans la limite", fg='green')
        distanceActuel = rep[1]
        libelleDistance.configure(text='Distance actuelle: ' + str(distanceActuel) + ' cm')

    if rep[0] == "OUT":
        libelleInfo.configure(text="En dehors de la limite", fg='red')
        distanceActuel = rep[1]
        libelleDistance.configure(text='Distance actuelle: ' + str(distanceActuel) + ' cm')

    connexion.flush()


def clicked():
    connexion.write(bytes("#" + entrySet.get(), 'utf-8'))
    print(bytes("#" + entrySet.get(), 'utf-8'))


window = Tk()
window.title('recording')
libelleSet = Label(window, text="Distance limit:")
libelleSet.grid(column=0, row=0)
entrySet = Entry(window, textvariable=limit)
entrySet.grid(column=0, row=1)
btnSet = Button(window, text="Envoyer", command=clicked)
btnSet.grid(column=0, row=2)
libelleDistance = Label(window, text='')
libelleDistance.grid(column=0, row=3)
libelleInfo = Label(window, text='')
libelleInfo.grid(column=0, row=4)
window.update()

try:
    connexion.open()
except Exception as error:
    pass

while True:
    lecture()
    window.update()
connexion.close()
