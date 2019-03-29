function clean(varargin)

	if nargin == 0
		clean d
		clean m
		clean t
	elseif strcmp(varargin, 'd') % data
		delete *.mat
		delete *.root
		delete *.xml
	elseif strcmp(varargin, 'm') % mex
		delete *.mexglx
		delete *.mexw32
	elseif strcmp(varargin, 't') % temp
		delete *.ilk
		delete *.log
		delete *.pdb
		delete *.txt
	end

end
