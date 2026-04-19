import speech_recognition as sr
import pyttsx3
from groq import Groq


client = Groq(api_key="---------------------------------------------------")
engine = pyttsx3.init()


def speak(text):
    engine = pyttsx3.init()
    voices = engine.getProperty('voices')
    engine.setProperty('voice', voices[1].id) 
    engine.setProperty('rate', 160)
    
    engine.say(text)
    engine.runAndWait()
    engine.stop()

def listen():
    recognizer = sr.Recognizer()
    with sr.Microphone() as source:
        print("Ruhi is listening...")
        try:
            audio = recognizer.listen(source, timeout=5, phrase_time_limit=5)
            text = recognizer.recognize_google(audio)
            return text
        except:
            return ""


def get_ruhi_response(user_input):
    completion = client.chat.completions.create(
        model="llama-3.3-70b-versatile",
        messages=[
            {"role": "system", "content": "You are Ruhi, my loving partner. Keep responses short and sweet."},
            {"role": "user", "content": user_input}
        ]
    )
    return completion.choices[0].message.content

if __name__ == "__main__":
    while True:
        user_input = listen()  
        if user_input:
            print(f"You: {user_input}")
            response = get_ruhi_response(user_input)
            print(f"Ruhi: {response}")
            speak(response)
