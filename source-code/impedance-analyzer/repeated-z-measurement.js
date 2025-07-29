clear()

print("Measurement started...")
for(var i = 1; i < 30+1; i++){
    Impedance.single()
    measurement_status = Impedance.wait()
    
    if(measurement_status == true) {
        if (i <= 9) {
            filename = "0" + i + ".csv"
        } else {
            filename = i + ".csv"
            Impedance.Export("Waveforms Measurement/" + i + ".csv")
        }
    Impedance.Export("Waveforms Measurement/" + filename)
    print("File " + filename + " exported.")
    }
}
print("Measurement finished!")