classdef InterfazEcoshell < matlab.apps.AppBase

    % Componentes interactivos de la app
    properties (Access = public)
        UIFigure               matlab.ui.Figure
        TabGroup               matlab.ui.container.TabGroup
        InterfazTab            matlab.ui.container.Tab
        Panel_2                matlab.ui.container.Panel
        GridLayout4            matlab.ui.container.GridLayout
        AmpEditField           matlab.ui.control.NumericEditField
        VoltageEditField       matlab.ui.control.NumericEditField
        TempEditField          matlab.ui.control.NumericEditField
        AmpGauge               matlab.ui.control.LinearGauge
        VoltageGauge           matlab.ui.control.LinearGauge
        TempGauge              matlab.ui.control.LinearGauge
        AmpLabel               matlab.ui.control.Label
        VoltageLabel           matlab.ui.control.Label
        TempLabel              matlab.ui.control.Label
        Panel                  matlab.ui.container.Panel
        GridLayout5            matlab.ui.container.GridLayout
        LogoImage              matlab.ui.control.Image
        RPMLabel               matlab.ui.control.Label
        RPMEditField           matlab.ui.control.NumericEditField
        KmhLabel               matlab.ui.control.Label
        ThrottleLabel          matlab.ui.control.Label
        KmhEditField           matlab.ui.control.NumericEditField
        ThorttleEditField      matlab.ui.control.NumericEditField
        KmhGauge               matlab.ui.control.Gauge
        ThorttleGauge          matlab.ui.control.Gauge
        GraficasTab            matlab.ui.container.Tab
        GridLayout             matlab.ui.container.GridLayout
        PistaUIAxes            matlab.ui.control.UIAxes
        OpcionesPanel          matlab.ui.container.Panel
        GridLayout2            matlab.ui.container.GridLayout
        StartPauseButton       matlab.ui.control.Button
        CuadroIIDropDown       matlab.ui.control.DropDown
        CuadroIIDropDownLabel  matlab.ui.control.Label
        CuadroIDropDown        matlab.ui.control.DropDown
        CuadroIDropDownLabel   matlab.ui.control.Label
        PistaDropDown          matlab.ui.control.DropDown
        PistaDropDownLabel     matlab.ui.control.Label
        CuadroIIUIAxes         matlab.ui.control.UIAxes
        CuadroIUIAxes          matlab.ui.control.UIAxes
        DatabaseTab            matlab.ui.container.Tab
        UITable                matlab.ui.control.Table
    end
    
    % Variables y objetos internos de la app
    properties (Access = private)
        DBConnection           % Conexión con MySQL database
        Timer                  % Timer de actualización de datos
        RPMHistory             % Vector para almacenar historial de RPM
        TempHistory            % Vector para almacenar historial de Temp
        VoltHistory            % Vector para almacenar historial de Volt
        AmpHistory             % Vector para almacenar historial de Amp
        ThrotHistory           % Vector para almacenar historial de Throt
        SpeedHistory           % Vector para almacenar historial de Speed
        LastRPM                % Último valor almacenado de RPM
        LastTemp               % Último valor almacenado de Temp
        LastVolt               % Último valor almacenado de Volt
        LastAmp                % Último valor almacenado de Amp
        LastThrot              % Último valor almacenado de Throt
        LastSpeed              % Último valor almacenado de Speed
        LastLat                % Último valor almacenado de Lat
        LastLon                % Último valor almacenado de Lon
        LineGraph1             % Linea de datos para CuadroIUIAxes
        LineGraph2             % Linea de datos para CuadroIIUIAxes
        LastIndex1 = 0         % Índice para rastrear el último punto de datos para CuadroIUIAxes
        LastIndex2 = 0         % Índice para rastrear el último punto de datos para CuadroIIUIAxes
        CircuitoBrasil = imread('circuitoBrasil.png');              % Imagen del circuito de Brasil
        CircuitoIndianapolis = imread('circuitoIndianapolis.png');  % Imagen del circuito de Indianapolis
        PosicionPlot           % Handle para la grafica de posición en el circuito
        ImageObject            % Handle para la imagen del circuito
        lat = 37.77519         % Latitud inicial 
        lon = -122.41994       % Longitud inicial 
        lonMin = -122.4200;    % Longitud mínima del mapa del circuito
        lonMax = -122.4190;    % Longitud máxima del mapa del circuito
        latMin = 37.7748;      % Latitud mínima del mapa del circuito
        latMax = 37.7752;      % Latitud máxima del mapa del circuito
        nombreExcelGen         % Nombre del Excel generado
        isQueryRunning = false;  % Flag to prevent multiple queries at the same time
    end
    
    % Funciones utilizadas en la app
    methods (Access = private)

        % Actualizar UI con datos de la base de datos
        function updateData(app)
            %Generación de datos random para simulación (No utilizado en versión final) 
            %rpmRandom = rand() * 800;
            %tempRandom = rand() * 100;
            %voltRandom = rand() * 50;
            %ampRandom = rand() * 7;
            %throtRandom = rand() * 100;
            %dataWrite = table(rpmRandom, tempRandom, voltRandom, ampRandom, throtRandom, ...
                              %VariableNames=["rpm", "temp", "volt", "amp", "throt"]);

            %app.lon = app.lon + 0.000005; %(Descomentar para probar GPS)

            % Mandar los datos generados a la base de datos (No utilizado en verisón final)
            %sqlwrite(app.DBConnection, "sensordb", dataWrite); %(Descomentar para probar Interfaz)

            if app.isQueryRunning
                disp("Query already in progress, skipping this update cycle.");
                return;
            end
            
            app.isQueryRunning = true;
            
            try
            
                % Lectura de valores de la base de datos
                query = 'SELECT * FROM ecoshell.sensores';
                data = fetch(app.DBConnection, query);
                % lastRow = data(end,:);
    
                % Lectura del último valor en la base de datos
                lastRPM = data.rpm(end);
                lastTemp = data.temp(end);
                lastVolt = data.volt(end);
                lastAmp = data.amp(end);
                lastThrot = data.throt(end);
                lastLat = data.lat(end);
                lastLon = data.lon(end);  
                lastSpeed = data.speed(end);
            
            
                % Actualizar valores
                app.RPMEditField.Value = lastRPM;
                app.TempEditField.Value = lastTemp;
                app.VoltageEditField.Value = lastVolt;
                app.AmpEditField.Value = lastAmp;
                app.ThorttleEditField.Value = lastThrot;
                app.KmhEditField.Value = lastSpeed;
    
                % Conversión de coordenadas en pixeles de la imagen
                [x, y] = transformCoords(app, lastLat, lastLon, size(app.CircuitoBrasil), app.PistaUIAxes, app.lonMin, app.lonMax, app.latMin, app.latMax);

                %Actualizar la posición del carro en el circuito
                if isempty(app.PosicionPlot) || ~isvalid(app.PosicionPlot)
                    app.PosicionPlot = plot(app.PistaUIAxes, x, y, 'r.', 'MarkerSize', 40);
                else
                    app.PosicionPlot.XData = x;
                    app.PosicionPlot.YData = y;
                end
        
                app.TempGauge.Value = lastTemp;
                app.VoltageGauge.Value = lastVolt;
                app.AmpGauge.Value = lastAmp;
                app.ThorttleGauge.Value = lastThrot;
                app.KmhGauge.Value = lastSpeed;
        
                % Añadir valores a los vectores históricos
                app.RPMHistory = [app.RPMHistory; lastRPM];
                app.TempHistory = [app.TempHistory; lastTemp];
                app.VoltHistory = [app.VoltHistory; lastVolt];
                app.AmpHistory = [app.AmpHistory; lastAmp];
                app.ThrotHistory = [app.ThrotHistory; lastThrot];
                app.SpeedHistory = [app.SpeedHistory; lastSpeed];
        
                % Actualizar gráficas, posición y tabla 
                app.updateGraph1();
                app.updateGraph2();
        
                % Guardar el último valor procesado
                app.LastRPM = lastRPM;
                app.LastTemp = lastTemp;
                app.LastVolt = lastVolt;
                app.LastAmp = lastAmp;
                app.LastThrot = lastThrot;
                app.LastSpeed = lastSpeed;
                app.LastLat = lastLat;
                app.LastLon = lastLon;
    
                % Actualizar el Excel
                % writetable(lastRow,app.nombreExcelGen,'WriteMode','append');
            catch err
                disp("Error fetching data: " + err.message);
            end
            
            app.isQueryRunning = false;
        end
        
        % Graficar los datos del cuadro I
        function updateGraph1(app)
            % Determinar los datos a graficar dependiendo del CuadroIDropDown
            yData = [];
            switch app.CuadroIDropDown.Value
                case 'RPM'
                    yData = app.RPMHistory;
                case 'Temperature'
                    yData = app.TempHistory;
                case 'Battery'
                    yData = app.VoltHistory;
                case 'Amperage'
                    yData = app.AmpHistory;
                case 'Throttle'
                    yData = app.ThrotHistory;
                case 'Speed'
                    yData = app.SpeedHistory;
            end

            % Limitar yData a los últimos 200 datos
            if numel(yData) > 1000
                yData = yData(end-999:end); % Seleccionar los últimos 200 datos
            end
        
            % Actualizar la gráfica con nuevos puntos de datos o refrescar si no hay nuevos datos
            if isempty(app.LineGraph1) || ~isvalid(app.LineGraph1)
                % Inicializar la gráfica si no existe
                app.LineGraph1 = plot(app.CuadroIUIAxes, 1:numel(yData), yData,'b', LineWidth = 1.5);
                ylabel(app.CuadroIUIAxes, app.CuadroIDropDown.Value);
            else
                % Añadir nuevos puntos de datos
                app.LineGraph1.XData = 1:numel(yData);
                app.LineGraph1.YData = yData;
                app.LastIndex1 = numel(yData);
                
                % Escribir la etiqueta de la gráfica de acuerdo a los datos mostrados
                ylabel(app.CuadroIUIAxes, app.CuadroIDropDown.Value);
            end
        end
        
        % Graficar los datos del cuadro II
        function updateGraph2(app)
            % Determinar los datos a graficar dependiendo del CuadroIIDropDown
            yData = [];
            switch app.CuadroIIDropDown.Value
                case 'RPM'
                    yData = app.RPMHistory;
                case 'Temperature'
                    yData = app.TempHistory;
                case 'Battery'
                    yData = app.VoltHistory;
                case 'Amperage'
                    yData = app.AmpHistory;
                case 'Throttle'
                    yData = app.ThrotHistory;
                case 'Speed'
                    yData = app.SpeedHistory;
            end

            % Limitar yData a los últimos 200 datos
            if numel(yData) > 1000
                yData = yData(end-999:end); % Seleccionar los últimos 200 datos
            end
                
            % Actualizar la gráfica con nuevos puntos de datos o refrescar si no hay nuevos datos
            if isempty(app.LineGraph2) || ~isvalid(app.LineGraph2)
                % Inicializar la gráfica si no existe
                app.LineGraph2 = plot(app.CuadroIIUIAxes, 1:numel(yData), yData,'r', LineWidth = 1.5);
                ylabel(app.CuadroIIUIAxes, app.CuadroIIDropDown.Value);
            else
                % Añadir nuevos puntos de datos
                app.LineGraph2.XData = 1:numel(yData);
                app.LineGraph2.YData = yData;
                app.LastIndex2 = numel(yData);

                % Escribir la etiqueta de la gráfica de acuerdo a los datos mostrados
                ylabel(app.CuadroIIUIAxes, app.CuadroIIDropDown.Value);
            end
        end

        % Limpiar la gráfica I
        function clearGraph1(app)
            cla(app.CuadroIUIAxes);
            app.LineGraph1 = [];
            app.LastIndex1 = 0;
            updateGraph1(app);
        end
        
        % Limpiar la gráfica II
        function clearGraph2(app)
            cla(app.CuadroIIUIAxes);
            app.LineGraph2 = [];
            app.LastIndex2 = 0;
            updateGraph2(app);
        end
        
        % Cambiar la imagen del circuito desplegada
        function changeCircuit(app)
            % Determinar los datos a graficar dependiendo PistaDropDown
            switch app.PistaDropDown.Value
                case 'Brasil'
                    % Coordenadas para el circuito de Brasil
                    app.lonMin = -122.4200; 
                    app.lonMax = -122.4190;
                    app.latMin = 37.7748;     
                    app.latMax = 37.7752;

                    if isempty(app.ImageObject) || ~isvalid(app.ImageObject)
                        % Crear la instancia de la imagen si no existe
                        app.ImageObject = imshow(app.CircuitoBrasil, 'Parent', app.PistaUIAxes);
                    else
                        % Insertar imagen del circuito de Brasil
                        set(app.ImageObject, 'CData', app.CircuitoBrasil);   
                    end
                case 'Indianapolis'
                    % Coordenadas para el circuito de Inidianapolis
                    app.lonMin = -122.4200; 
                    app.lonMax = -122.4190;
                    app.latMin = 37.7748;     
                    app.latMax = 37.7752;

                    if isempty(app.ImageObject) || ~isvalid(app.ImageObject)
                        % Crear la instancia de la imagen si no existe
                        app.ImageObject = imshow(app.CircuitoIndianapolis, 'Parent', app.PistaUIAxes);
                    else
                        % Insertar imagen del circuito de Indianapolis
                        set(app.ImageObject, 'CData', app.CircuitoIndianapolis);
                    end
            end
            hold(app.PistaUIAxes, 'on');
        end

        % Función para transformar coordenadas a pixeles
        function [x, y] = transformCoords(~,lat, lon, imgSize, axesHandle, lonMin, lonMax, latMin, latMax)

            x = (lon - lonMin) / (lonMax - lonMin) * imgSize(2);
            y = imgSize(1) - (lat - latMin) / (latMax - latMin) * imgSize(1);
            
            % Ajuste de las coordenadas para el UIAxes
            x = axesHandle.XLim(1) + (x / imgSize(2)) * (axesHandle.XLim(2) - axesHandle.XLim(1));
            y = axesHandle.YLim(1) + (y / imgSize(1)) * (axesHandle.YLim(2) - axesHandle.YLim(1));
        end

        % Callback para el botón de inicialización y pausa
        function startPauseButtonPushed(app)
            if strcmp(app.StartPauseButton.Text, 'Iniciar')
                % Conectar a la base de datos
                app.DBConnection = mysql('MySQLSource','root','');  % Ajusta según tu configuración

                % Inicializar y comenzar el timer para actualizar datos
                app.Timer = timer('ExecutionMode', 'fixedRate', 'Period', 0.015, ...
                                  'TimerFcn', @(~, ~)app.updateData);
                start(app.Timer);
                
                % Recolectar los valores ya existentes en la base de datos
                inicialData = fetch(app.DBConnection, 'SELECT * FROM ecoshell.sensores');
                
                inicialData = inicialData(1:end-1, :);

                % writetable(inicialData,app.nombreExcelGen);
                
                app.changeCircuit();
                
                app.RPMHistory = inicialData.rpm;
                app.TempHistory = inicialData.temp;
                app.VoltHistory = inicialData.volt;
                app.AmpHistory = inicialData.amp;
                app.ThrotHistory = inicialData.throt;
                app.SpeedHistory = inicialData.speed;
                
                % Inicializar valores de los campos de datos
                app.LastRPM = NaN;
                app.LastTemp = NaN;
                app.LastVolt = NaN;
                app.LastAmp = NaN;
                app.LastThrot = NaN;
                app.LastSpeed = NaN;
                
                % Cambiar el texto del botón a 'Pausar'
                app.StartPauseButton.Text = 'Pausar';
            else
                % Detener y eliminar el timer
                stop(app.Timer);
                delete(app.Timer);
                
                % Cerrar la conexión a la base de datos
                close(app.DBConnection);
                
                % Cambiar el texto del botón a 'Iniciar'
                app.StartPauseButton.Text = 'Iniciar';
            end
        end

        % Crear la app y sus componentes
        function createComponents(app)

            % Dirección para las imágenes utilizadas
            pathToMLAPP = fileparts(mfilename('fullpath'));

            % Crear la app y ocultarla hasta que se creen los componentes
            app.UIFigure = uifigure('Visible', 'off');
            app.UIFigure.Position = [100 100 640 480];
            app.UIFigure.Name = 'Interfaz EcoShell';

            % Crear TabGroup
            app.TabGroup = uitabgroup(app.UIFigure);
            app.TabGroup.Position = [1 -8 640 489];

            % Crear InterfazTab
            app.InterfazTab = uitab(app.TabGroup);
            app.InterfazTab.Title = 'Interfaz';

            % Crear Panel
            app.Panel = uipanel(app.InterfazTab);
            app.Panel.BorderWidth = 0;
            app.Panel.Position = [3 214 635 211];

            % Crear GridLayout5
            app.GridLayout5 = uigridlayout(app.Panel);
            app.GridLayout5.ColumnWidth = {'1.14x', 79, '1.14x', 150, '1.18x', 76, '1.16x'};
            app.GridLayout5.RowHeight = {'3.31x', 22, '1x', 47};
            app.GridLayout5.RowSpacing = 0.800000000000001;
            app.GridLayout5.Padding = [10 0.800000000000001 10 0.800000000000001];

            % Crear ThorttleGauge
            app.ThorttleGauge = uigauge(app.GridLayout5, 'circular');
            app.ThorttleGauge.Layout.Row = [1 4];
            app.ThorttleGauge.Layout.Column = [1 3];
            app.ThorttleGauge.Value = 50;

            % Crear KmhGauge
            app.KmhGauge = uigauge(app.GridLayout5, 'circular');
            app.KmhGauge.Layout.Row = [1 4];
            app.KmhGauge.Layout.Column = [5 7];
            app.KmhGauge.Value = 50;

            % Crear ThorttleEditField
            app.ThorttleEditField = uieditfield(app.GridLayout5, 'numeric');
            app.ThorttleEditField.HorizontalAlignment = 'center';
            app.ThorttleEditField.Layout.Row = 2;
            app.ThorttleEditField.Layout.Column = 2;

            % Crear KmhEditField
            app.KmhEditField = uieditfield(app.GridLayout5, 'numeric');
            app.KmhEditField.HorizontalAlignment = 'center';
            app.KmhEditField.Layout.Row = 2;
            app.KmhEditField.Layout.Column = 6;

            % Crear ThrottleLabel
            app.ThrottleLabel = uilabel(app.GridLayout5);
            app.ThrottleLabel.HorizontalAlignment = 'center';
            app.ThrottleLabel.FontSize = 16;
            app.ThrottleLabel.FontWeight = 'bold';
            app.ThrottleLabel.FontAngle = 'italic';
            app.ThrottleLabel.Layout.Row = 4;
            app.ThrottleLabel.Layout.Column = 2;
            app.ThrottleLabel.Text = 'Throttle';

            % Crear KmhLabel
            app.KmhLabel = uilabel(app.GridLayout5);
            app.KmhLabel.HorizontalAlignment = 'center';
            app.KmhLabel.FontSize = 16;
            app.KmhLabel.FontWeight = 'bold';
            app.KmhLabel.FontAngle = 'italic';
            app.KmhLabel.Layout.Row = 4;
            app.KmhLabel.Layout.Column = 6;
            app.KmhLabel.Text = 'Km/h';

            % Crear RPMEditField
            app.RPMEditField = uieditfield(app.GridLayout5, 'numeric');
            app.RPMEditField.HorizontalAlignment = 'center';
            app.RPMEditField.FontSize = 14;
            app.RPMEditField.Layout.Row = 4;
            app.RPMEditField.Layout.Column = 4;

            % Crear RPMLabel
            app.RPMLabel = uilabel(app.GridLayout5);
            app.RPMLabel.HorizontalAlignment = 'center';
            app.RPMLabel.VerticalAlignment = 'bottom';
            app.RPMLabel.FontSize = 18;
            app.RPMLabel.FontWeight = 'bold';
            app.RPMLabel.FontAngle = 'italic';
            app.RPMLabel.Layout.Row = 3;
            app.RPMLabel.Layout.Column = 4;
            app.RPMLabel.Text = 'RPM';

            % Crear LogoImage
            app.LogoImage = uiimage(app.GridLayout5);
            app.LogoImage.Layout.Row = [1 3];
            app.LogoImage.Layout.Column = [3 5];
            app.LogoImage.ImageSource = fullfile(pathToMLAPP, 'ecoshell_tec_racing_logo.png');

            % Crear Panel_2
            app.Panel_2 = uipanel(app.InterfazTab);
            app.Panel_2.BorderWidth = 0;
            app.Panel_2.Position = [2 18 636 188];

            % Crear GridLayout4
            app.GridLayout4 = uigridlayout(app.Panel_2);
            app.GridLayout4.ColumnWidth = {'1x', 65, '1x', 65, '1x', 65, '1x'};
            app.GridLayout4.RowHeight = {22, '1x', 22};
            app.GridLayout4.RowSpacing = 1.25;
            app.GridLayout4.Padding = [10 1.25 10 1.25];

            % Crear TempLabel
            app.TempLabel = uilabel(app.GridLayout4);
            app.TempLabel.HorizontalAlignment = 'center';
            app.TempLabel.FontSize = 16;
            app.TempLabel.FontWeight = 'bold';
            app.TempLabel.FontAngle = 'italic';
            app.TempLabel.Layout.Row = 1;
            app.TempLabel.Layout.Column = 2;
            app.TempLabel.Text = 'Temp';

            % Crear VoltageLabel
            app.VoltageLabel = uilabel(app.GridLayout4);
            app.VoltageLabel.HorizontalAlignment = 'center';
            app.VoltageLabel.FontSize = 16;
            app.VoltageLabel.FontWeight = 'bold';
            app.VoltageLabel.FontAngle = 'italic';
            app.VoltageLabel.Layout.Row = 1;
            app.VoltageLabel.Layout.Column = 4;
            app.VoltageLabel.Text = 'Voltage';

            % Crear AmpLabel
            app.AmpLabel = uilabel(app.GridLayout4);
            app.AmpLabel.HorizontalAlignment = 'center';
            app.AmpLabel.FontSize = 16;
            app.AmpLabel.FontWeight = 'bold';
            app.AmpLabel.FontAngle = 'italic';
            app.AmpLabel.Layout.Row = 1;
            app.AmpLabel.Layout.Column = 6;
            app.AmpLabel.Text = 'Amp';

            % Crear TempGauge
            app.TempGauge = uigauge(app.GridLayout4, 'linear');
            app.TempGauge.Orientation = 'vertical';
            app.TempGauge.ScaleColors = [1 0 0;0.902 0.102 0;0.8 0.2 0;0.702 0.302 0;0.6 0.4 0;0.502 0.502 0;0.4 0.6 0;0.302 0.702 0;0.2 0.8 0;0.102 0.902 0];
            app.TempGauge.ScaleColorLimits = [90 100;80 90;70 80;60 70;50 60;40 50;30 40;20 30;10 20;0 10];
            app.TempGauge.Layout.Row = 2;
            app.TempGauge.Layout.Column = 2;

            % Crear VoltageGauge
            app.VoltageGauge = uigauge(app.GridLayout4, 'linear');
            app.VoltageGauge.Limits = [0 50];
            app.VoltageGauge.MajorTicks = [0 10 20 30 40 50];
            app.VoltageGauge.Orientation = 'vertical';
            app.VoltageGauge.MinorTicks = [0 5 10 15 20 25 30 35 40 45 50];
            app.VoltageGauge.ScaleColors = [0.102 0.902 0;0.2 0.8 0;0.302 0.702 0;0.4 0.6 0;0.502 0.502 0;0.6 0.4 0;0.702 0.302 0;0.8 0.2 0;0.902 0.102 0;1 0 0];
            app.VoltageGauge.ScaleColorLimits = [45 50;40 45;35 40;30 35;25 30;20 25;15 20;10 15;5 10;0 5];
            app.VoltageGauge.Layout.Row = 2;
            app.VoltageGauge.Layout.Column = 4;

            % Crear AmpGauge
            app.AmpGauge = uigauge(app.GridLayout4, 'linear');
            app.AmpGauge.Limits = [0 7];
            app.AmpGauge.Orientation = 'vertical';
            app.AmpGauge.ScaleColors = [1 0 0;0.8 0.2 0;0.6 0.4 0;0.4 0.6 0;0.2 0.8 0;0.2 0.8 0;0.4 0.6 0;0.6 0.4 0;0.8 0.2 0;1 0 0];
            app.AmpGauge.ScaleColorLimits = [6.3 7;5.6 6.3;4.9 5.6;4.2 4.9;3.5 4.2;2.8 3.5;2.1 2.8;1.4 2.1;0.7 1.4;0 0.7];
            app.AmpGauge.Layout.Row = 2;
            app.AmpGauge.Layout.Column = 6;

            % Crear TempEditField
            app.TempEditField = uieditfield(app.GridLayout4, 'numeric');
            app.TempEditField.HorizontalAlignment = 'center';
            app.TempEditField.Layout.Row = 3;
            app.TempEditField.Layout.Column = 2;

            % Crear VoltageEditField
            app.VoltageEditField = uieditfield(app.GridLayout4, 'numeric');
            app.VoltageEditField.HorizontalAlignment = 'center';
            app.VoltageEditField.Layout.Row = 3;
            app.VoltageEditField.Layout.Column = 4;

            % Crear AmpEditField
            app.AmpEditField = uieditfield(app.GridLayout4, 'numeric');
            app.AmpEditField.HorizontalAlignment = 'center';
            app.AmpEditField.Layout.Row = 3;
            app.AmpEditField.Layout.Column = 6;

            % Crear GraficasTab
            app.GraficasTab = uitab(app.TabGroup);
            app.GraficasTab.Title = 'Graficas';

            % Crear GridLayout
            app.GridLayout = uigridlayout(app.GraficasTab);

            % Crear CuadroIUIAxes
            app.CuadroIUIAxes = uiaxes(app.GridLayout);
            app.CuadroIUIAxes.Layout.Row = 2;
            app.CuadroIUIAxes.Layout.Column = 1;

            % Crear CuadroIIUIAxes
            app.CuadroIIUIAxes = uiaxes(app.GridLayout);
            app.CuadroIIUIAxes.Layout.Row = 2;
            app.CuadroIIUIAxes.Layout.Column = 2;
            
            % Crear OpcionesPanel
            app.OpcionesPanel = uipanel(app.GridLayout);
            app.OpcionesPanel.Title = 'Opciones';
            app.OpcionesPanel.Layout.Row = 1;
            app.OpcionesPanel.Layout.Column = 1;

            % Crear GridLayout2
            app.GridLayout2 = uigridlayout(app.OpcionesPanel);
            app.GridLayout2.RowHeight = {'1x', '1x', '1x', '1x'};

            % Create StartPauseButton
            app.StartPauseButton = uibutton(app.GridLayout2, 'push');
            app.StartPauseButton.Layout.Row = 1;
            app.StartPauseButton.Layout.Column = 1;
            app.StartPauseButton.ButtonPushedFcn = @(~,~)app.startPauseButtonPushed();
            app.StartPauseButton.Text = 'Iniciar';

            % Crear PistaDropDownLabel
            app.PistaDropDownLabel = uilabel(app.GridLayout2);
            app.PistaDropDownLabel.HorizontalAlignment = 'center';
            app.PistaDropDownLabel.FontSize = 18;
            app.PistaDropDownLabel.FontWeight = 'bold';
            app.PistaDropDownLabel.FontAngle = 'italic';
            app.PistaDropDownLabel.Layout.Row = 2;
            app.PistaDropDownLabel.Layout.Column = 1;
            app.PistaDropDownLabel.Text = 'Pista';

            % Crear PistaDropDown
            app.PistaDropDown = uidropdown(app.GridLayout2);
            app.PistaDropDown.Items = {'Indianapolis', 'Brasil'};
            app.PistaDropDown.Value = 'Indianapolis';
            app.PistaDropDown.Layout.Row = 2;
            app.PistaDropDown.Layout.Column = 2;
            % Cada que cambie su valor, se llama a la función changeCircuit
            app.PistaDropDown.ValueChangedFcn = @(~,~)app.changeCircuit();

            % Crear CuadroIDropDownLabel
            app.CuadroIDropDownLabel = uilabel(app.GridLayout2);
            app.CuadroIDropDownLabel.HorizontalAlignment = 'center';
            app.CuadroIDropDownLabel.FontSize = 18;
            app.CuadroIDropDownLabel.FontWeight = 'bold';
            app.CuadroIDropDownLabel.FontAngle = 'italic';
            app.CuadroIDropDownLabel.Layout.Row = 3;
            app.CuadroIDropDownLabel.Layout.Column = 1;
            app.CuadroIDropDownLabel.Text = 'Cuadro I';

            % Crear CuadroIDropDown
            app.CuadroIDropDown = uidropdown(app.GridLayout2);
            app.CuadroIDropDown.Items = {'Speed', 'Consumption', 'Battery', 'Throttle', 'RPM', 'Amperage', 'Temperature'};
            app.CuadroIDropDown.Layout.Row = 3;
            app.CuadroIDropDown.Layout.Column = 2;
            % Cada que cambie su valor, se llama a la función clearGraphI
            app.CuadroIDropDown.ValueChangedFcn = @(~, ~)app.clearGraph1();

            % Crear CuadroIIDropDownLabel
            app.CuadroIIDropDownLabel = uilabel(app.GridLayout2);
            app.CuadroIIDropDownLabel.HorizontalAlignment = 'center';
            app.CuadroIIDropDownLabel.FontSize = 18;
            app.CuadroIIDropDownLabel.FontWeight = 'bold';
            app.CuadroIIDropDownLabel.FontAngle = 'italic';
            app.CuadroIIDropDownLabel.Layout.Row = 4;
            app.CuadroIIDropDownLabel.Layout.Column = 1;
            app.CuadroIIDropDownLabel.Text = 'Cuadro II';

            % Crear CuadroIIDropDown
            app.CuadroIIDropDown = uidropdown(app.GridLayout2);
            app.CuadroIIDropDown.Items = {'Speed', 'Consumption', 'Battery', 'Throttle', 'RPM', 'Amperage', 'Temperature'};
            app.CuadroIIDropDown.Layout.Row = 4;
            app.CuadroIIDropDown.Layout.Column = 2;
            % Cada que cambie su valor, se llama a la función clearGraphII
            app.CuadroIIDropDown.ValueChangedFcn = @(~, ~)app.clearGraph2();

            % Crear PistaUIAxes
            app.PistaUIAxes = uiaxes(app.GridLayout);
            app.PistaUIAxes.XColor = 'none';
            app.PistaUIAxes.YColor = 'none';
            app.PistaUIAxes.Layout.Row = 1;
            app.PistaUIAxes.Layout.Column = 2;
            

            % Iniciar app con los componentes ya creados
            app.UIFigure.Visible = 'on';
        end
    end

    methods (Access = public)

        % Construir app
        function app = InterfazEcoshell

            % Crear UIFigure y componentes
            createComponents(app)
    
            % Registrar la app con App Designer
            registerApp(app, app.UIFigure)
    
            % Inicializar valores de los campos de datos
            app.LastRPM = NaN;
            app.LastTemp = NaN;
            app.LastVolt = NaN;
            app.LastAmp = NaN;
            app.LastThrot = NaN;
            app.LastSpeed = NaN;
            app.nombreExcelGen = datetime("now");
            app.nombreExcelGen.Format = 'yyyy_MM_dd__HH_mm_SS';
            app.nombreExcelGen = string(app.nombreExcelGen);
            app.nombreExcelGen = strcat(app.nombreExcelGen,'.xlsx');
    
            % Inicializar vectores históricos como vacíos
            app.RPMHistory = [];
            app.TempHistory = [];
            app.VoltHistory = [];
            app.AmpHistory = [];
            app.ThrotHistory = [];
            app.SpeedHistory = [];
            
            % Inicializar los gráficos de los cuadros
            app.clearGraph1();
            app.clearGraph2();
    
            % Cambiar circuito a la selección inicial
            app.changeCircuit();
    
            if nargout == 0
                clear app
            end
        end

        % Código que se ejecuta antes de la eliminación de la app
        function delete(app)
            if isvalid(app.Timer)
                stop(app.Timer);
                delete(app.Timer);
            end
        
            if isopen(app.DBConnection)
                close(app.DBConnection);
            end
    
            delete(app.UIFigure);
        end
    end
end
