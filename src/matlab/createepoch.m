function [outputArg1] = createepoch(timeAsString)
%UNTITLED Summary of this function goes here
%   Detailed explanation goes here


newTimeAsString = strrep(timeAsString,'_',' ');
t1=datetime(newTimeAsString);
outputArg1=posixtime(t1);

end

