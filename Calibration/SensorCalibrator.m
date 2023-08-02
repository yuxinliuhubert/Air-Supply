function SensorCalibrator
%% Initial Reset
% Clear everything
close all; clear;
% clear all
% reset all ports; otherwise might be unable to connect to port

% create our clean up object for interrupt
cleanupObj = onCleanup(@cleanMeUp);

%% User Control
% Linear least square method is used in this code.

% NAME THE TEST FIRST
% The code will read from the previous data, or establish a new file if no
% data present.
% MUST CHANGE NAME OR DELETE PREVIOUS FILE IF DIFFERENT NUMBER OF SENSORS REPORT DATA

% Name the file (will be used in data logging and graph titles)
testDevice = 'PT'; % Use 'FL' if for flow meter, 'PT' for pressure transducer
date = 'Aug01';
iteration = 1; % in case you want multiple calibration runs for the same device on same day

% How many sensors are you reporting each time? (match with Arduino output)
dataLength = 1;

% How many data points do you want to use to calculate (if use 5,only the last 5 data points will be kept upon stopping the
% program)
dataPointNum = 5;


% you don't have to change fileName and folderName, as they are automated, unless you want
% to add anything else to it
fileName = sprintf('%s_%s_Calibration_%d',testDevice,date,iteration);

% NAME THE FOLDER YOU WANT THE TEST TO BE IN
folderName = sprintf('%s_%s_Calibration',date,testDevice);


%% Automated Process Starts here
errorOccured = false;
try
    ports = serialportlist;
    num_ports = length(ports);
    for k = 1:num_ports
        fprintf('%d. %s\n', k, ports(k))
    end

    % Ask the user to input the COM port number
    port_num = input('Please choose the COM port you want to use (type the listing number): ');
    device = ports(port_num);

    % Make sure it matches the baud rate on your board's output
    baudRate = 115200;


    dataFileExist = 0;
    arrayMatch = 1;
    prevArray = [];
    if exist([fileName,'.xls'])
        dataFileExist = 1;
        prevTable = readtable([fileName,'.xls']);
        prevArray = table2array(prevTable);
        prevArray = prevArray(3:end,:);

        [~,colLength] = size(prevArray);
        if dataLength ~= colLength-1
            ME = MException('MyComponent:newAndOldDataStructureMismatch',['The' ...
                'file you are reading from has a different number of sensor being logged,' ...
                'change the name of the file or delete the original file']);
            arrayMatch = 0;
            throw(ME)
        end

    end


    % set up dynamic table columns
    dataLabels = [];
    for n = 1:dataLength
        eachLabel = convertCharsToStrings({[testDevice,'_',num2str(n)]});
        dataLabels = [dataLabels,eachLabel];

    end
    eachLabel = convertCharsToStrings({[testDevice,'_','Readings']});
    dataLabels = [dataLabels,eachLabel];


    finalArray = [];
    reading = [];


    rawData = [];
    i = 1;

    try
        s = serialport(device,baudRate);
    catch
        availablePorts = convertStringsToChars(serialportlist("all"));
        [~,numOfPorts] = size(availablePorts);
        portNames = [];
        for n = 1:numOfPorts
            if n == 1
                portNames = availablePorts{n};
            else
                portNames = [portNames, ', ',availablePorts{n}];
            end
        end

        ME = MException('MyComponent:fopenFailed',['Failed to open the serial ' ...
            'port, check the serial port name, close up Arduino''s serial monitor, or reconnect the device. Available ports: ' portNames]);
        throw(ME)
    end
    if testDevice == "PT "
        write(s,0,"double");
    elseif testDevice == "FL "
        write(s,1,"double");
    end
    flush(s);

    timeout_duration = 5;
        tic;  % start timer
        while toc < timeout_duration
            data = str2num(readline(s));

            if length(data) == dataLength
                break;

            end
        end

        if toc >= timeout_duration
            error('Input format error. Number of input expected: %d; detected: %d',dataLength, length(data));
        end
        fprintf("Collecting Data Points. Please wait for a while, and click stop to go to next step");
        while(1)

            data = str2num(readline(s));
            length(data);

            if length(data) ~= dataLength
                warning('Number of input expected: %d; detected: %d',dataLength, length(data));
                continue;
            end

            rawData(i,:) = data;


            if i <= dataPointNum
                i = i+1;
            else 
               rawData(i-dataPointNum,:) = [];
            end
            rawData

        end

catch ME
         errorOccurred = true; % Declare errorOccurred in the same scope
         rethrow(ME); % Rethrow the error to stop execution and trigger cleanup
   
end
    function cleanMeUp()
        if exist('errorOccurred', 'var') && errorOccurred %#ok<MOCUP> 

        else
            % saves data to file (or could save to workspace)
            fprintf('saving test data as %s.xls\n',fileName)

            prompt = sprintf("What is the %s gauge reading? (Numbers only) \n",testDevice);
            reading = input(prompt);

            if ~exist(folderName, 'dir')
                mkdir(folderName);
                addpath(folderName);
                fprintf("test data folder created\n");
            else
                fprintf("folder already exists\n")
                addpath(folderName);
            end
            % calculate mean values

            for n = 1:dataLength
                meanArray(1,n) = mean(rmoutliers(rawData(:,n)));
            end
            processArray = [meanArray,reading];
            processArray = [prevArray;processArray];

            a = [];
            b = [];
            endsol = [];
            for j = 1:length(dataLabels)-1
                X = processArray(:,j);
                Y = processArray(:,end);
                try
                    % Convert warning to error
                    warning('error', 'MATLAB:polyfit:RepeatedPointsOrRescale');
                    coefficients = polyfit(X,Y,1)';
                    a = [a;coefficients(1)];
                    b = [b;coefficients(2)];
                    endsol = [endsol,coefficients];
                catch
                    error(sprintf("cannot establish linear best fit with the input you gave input. "))
                    if strcmp(ME.identifier, 'MATLAB:polyfit:RepeatedPointsOrRescale')
                        % Handle the situation, e.g., print a message or break the loop
                        disp('Polynomial is not unique; degree >= number of data points. Cannot establish linear best fit with the input.')
                    else
                        % If it's a different kind of error, rethrow it
                        rethrow(ME);
                    end
                end


            end

            finalArray = [[a',NaN];[b',NaN];processArray];

            testDataTable = array2table(finalArray,'VariableNames',dataLabels);

            fileString = fileName + ".xls";

            if dataFileExist == 1
                writetable(testDataTable,fileString,'WriteMode','overwrite')
                movefile(fileString,folderName);
            else
                writetable(testDataTable,fileName,"FileType","spreadsheet");
                movefile(fileString,folderName);

            end
            dataProcessingGraphing(processArray,endsol)

        end
    end

    function dataProcessingGraphing(array,solution)
        figure;
        set(gcf, 'PaperSize', [10 10]);
        plotNumber = length(array(1,:))-1;

        for k = 1:plotNumber
            nexttile
            sortedArray = sort(array(:,k));

            x = linspace(sortedArray(1),sortedArray(end),100);
            titleString = [testDevice,num2str(k)];
            plot(array(:,k),array(:,end),'o')
            title(titleString);
            hold on
            y = solution(1,k)*x+solution(2,k);

            plot(x,y)
            hold off

        end
    end

end


