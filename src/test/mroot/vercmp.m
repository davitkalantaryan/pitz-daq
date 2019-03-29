function result = vercmp(ver1, varargin)
% Compares two version numbers.
% 
% Example: 7.4.0.287
%   <major release>.<minor release>.<patch level>.<build>
%   Everything except the major release is optional.
%
% Note: Depends on explode.m
% 
% Arguments:
%     ver1 - Version number one
%     varargin - Version number two; default: version command output
% 
% Return values:
%     result - Index of part which differs:
%              0 if equal
%            > 0 if ver1 > ver2
%            < 1 if ver1 < ver2
%            NaN if ver1 | ver2 are not a version number
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% check args
	if isempty(varargin)
		[s, e, i, ver2] = matchVer(version);
	else
		ver2 = varargin{1};
	end
	
	if ~isVer(ver1) || ~isVer(ver2)
		result = NaN;
		return
	end
	
	% init result
	result = 0;
	
	% precalcs
	ver1 = explode(ver1, '.');
	ver2 = explode(ver2, '.');
	
	lV1 = length(ver1);
	lV2 = length(ver2);
	
	% loop over parts of ver1
	for i = 1:lV1
		if i > lV2 % ver1 longer than ver2
			result = i;
			return
		else
			nV1 = str2double(ver1{i});
			nV2 = str2double(ver2{i});
			
			if nV1 > nV2 % ver1 > ver2
				result =  i;
				return
			end
			
			if nV1 < nV2 % ver1 < ver2
				result = -i;
				return
			end
		end
	end
	
	if lV1 < lV2 % ver1 shorter than ver2
		result = -(lV1 + 1);
	end
end

function result = isVer(in)
	result = 1;
	
	if ~ischar(in)
		result = 0;
	elseif length(strfind(in, '.')) > 3
		result = 0;
	else
		[s, e] = matchVer(in);
		
		if s > 1 || e < length(in)
			result = 0;
		end
	end
end

function varargout = matchVer(in)
	[s, e, i, m] = regexp(in, '\d+(\.\d+)*', 'once');
	varargout = {s, e, i, m};
end
