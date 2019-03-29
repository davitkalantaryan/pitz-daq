function compile(varargin)
% Compiles several source files in one binary.
% 
% Arguments:
%     varargin - names of source files
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% set up directories
	workingDir = fileparts(mfilename('fullpath'));
	SRC_DIR = [ workingDir filesep 'src' filesep ];
	BIN_DIR = [ workingDir filesep 'private' filesep ];
	
	if isunix; ROOTSYS = '$ROOTSYS'; end
	if ispc;   ROOTSYS = 'H:\root';  end
	
	% prefix files with SRC_DIR
	varargin = cellfun(@(x) [SRC_DIR x], varargin, 'UniformOutput', 0);
	
	% compose flags
	C_FLAGS = { % compile flags
	    '-g' % debug symbols
	   ['-I' ROOTSYS filesep 'include'] % include path
	    '-DDEBUG' % define symbol
	}';
	L_FLAGS = { % link flags
	   ['-L' ROOTSYS filesep 'lib'] % link path
	    '-lCore -lCint -lRIO -lXMLIO -lNet -lSQL -lRMySQL -lRootAuth -lMathCore -lMatrix' % libs
	    '-outdir' % out dir
	     BIN_DIR
	}';
	
	if isunix % linux flags
		C_FLAGS = [ C_FLAGS'
		    '-DPATROL' % mpatrol only on linux
		    '-I$MPATROL/src'
		]';
		L_FLAGS = [ L_FLAGS'
		    '-L$MPATROL/build/unix'
		    '-lmpatrol -lmpalloc -lmpatrolmt -lbfd'
		]';
	end
	
	if ispc % windows flags
		C_FLAGS = [ C_FLAGS'
		    '-DPROFILE' % shiny only on windows
		    '-IH:\Shiny\include'
		]';
		L_FLAGS = [ L_FLAGS'
		    '-LH:\Shiny\lib'
		    '-lShiny'
		]';
	end
	
	% create compile string
	mx = sprintf('mex %s %s %s', ...
	    implode(C_FLAGS,  ' '),  ...
	    implode(L_FLAGS,  ' '),  ...
	    implode(varargin, ' ')   ...
	);
	
	% compile binary
	eval(mx);
end
