<?php
    
    // Datos del servidor y la base de datos
    $hostname = "localhost";
    $username = "root";
    $password = "";
    $database = "ecoshell";

    // Conexión a la base de datos
    $conn = mysqli_connect($hostname, $username, $password, $database);

    if (!$conn) {
        die("Error de conexión: " . mysqli_connect_error());
    }

    // Recolección y envío de los datos
    if (isset($_POST["rpmESP"]) || isset($_POST["tempESP"]) || isset($_POST["voltESP"]) || isset($_POST["ampESP"]) || isset($_POST["throtESP"]) || isset($_POST["latESP"]) || isset($_POST["lonESP"])) { 

        // Valores recibidos de la ESP
        $rpmPHP = $_POST["rpmESP"];
        $tempPHP = $_POST["tempESP"];
        $voltPHP = $_POST["voltESP"];
        $ampPHP = $_POST["ampESP"];
        $throtPHP = $_POST["throtESP"];
        $latPHP = $_POST["latESP"];
        $lonPHP = $_POST["lonESP"];

        // Set de instrucción para actualizar los datos en la db
        $instruccion = "INSERT INTO sensordatabase (rpm, temp, volt, amp, throt, lat, lon) VALUES ('$rpmPHP', '$tempPHP', '$voltPHP', '$ampPHP', '$throtPHP', '$latPHP', '$lonPHP')";

        if (mysqli_query($conn, $instruccion)) { 
            // Enviar los datos
            echo "Actualizado con éxito";
        } else {
            echo "Error: " . $instruccion . "<br>" . mysqli_error($conn);
        }
    } 

    // Cerrar la conexión
    mysqli_close($conn);
?>
