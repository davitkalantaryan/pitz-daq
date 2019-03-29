function [ofile, ifile] = pathinfo(filename)
% Splits the given ROOT path in its elements.
% 
% Arguments:
%     filename - A ROOT path:
% 
%         [[proto://usr [:pwd] @] path/] name.ext [#path]
% 
%         e.g. root://usr:pwd@localhost/~usr/test.root#foo/bar
% 
%       * proto: rootd or dbms
%       * usr, pwd: only for network related files; pwd optional
%       * '#' splits o- and ifile
% 
% Return values:
%     ofile - outer file; struct specifying the ROOT file:
%       * type: -1 = No ROOT, 0 = TFile, 1 = TXMLFile, 2 = TNetFile, 3 = TSQLFile
%       * path: Path of file
%       * usr and pwd: Login information
% 
%     ifile - inner file; struct with path in file
% 
% Copyright (C) 2008 Johannes Kissel<johannes.kissel@ipe.fzk.de>
% For the licensing terms see ./COPYING.

	% check for non-string argument
	if ~ischar(filename)
		error('Argument ''filename'' must contain a string.');
	end
	
	type = -1; % default: no ROOT file
	
	% check for network related file
	i = findstr(filename, '://');
	j = findstr(filename,  '@' );
	
	if ~isempty(j)
		proto = filename(1:(i - 1));
		
		% whitelists for rootd and dbms
		root = {
			'root'   % rootd
			'roots'  % rootd over SSH
			'rootk'  % rootd over Kerberos
		};
		dbms = { % add your dbms here (if supported by ROOTs TSQLServer)
			'mysql'  % MySQL
			'pgsql'  % PostgreSQL
			'oracle' % Oracle
		};
		
		if     find(cellfun(@(x) strcmpi(proto, x), root), 1)
			type = 2; % TNetFile
		elseif find(cellfun(@(x) strcmpi(proto, x), dbms), 1)
			type = 3; % TSQLFile
		else
			error('path:proto', 'Unsupported protocol: %s', proto);
		end
		
		% extract usr and pwd
		[usr, pwd] = strtok(filename((i + 3):(j - 1)), ':');
		
		if isempty(usr)
			error('path:login', 'No login information given.');
		end
		
		pwd      = pwd(2:end);
		filename = [filename(1:(i + 2)) filename((j + 1):end)];
	end
	
	% split filename to o- and ifile
	[opath, ipath]    = strtok(filename, '#');
	[path, name, ext] = fileparts(opath);
	
	% check for local file
	if type < 0 && ~strcmpi(ext, '.mat');
		if (isempty(path) || exist(path, 'dir')) && (~isempty(name) || ~isempty(ext))
			if exist(opath, 'file')
				fid = fopen(opath, 'rt');

				if fid < 0
					error('path:io', 'Error opening file: %s', opath);
				end

				r = native2unicode(fread(fid, 4)'); % first 4 bytes
				fclose(fid);

				if strcmp(r, 'root')
					type = 0; % TFile
				else
					try
						doc = xmlread(opath);
						r   = doc.getDocumentElement.getTagName; % root tag
						
						if strcmp(r, 'root')
							type = 1; % TXMLFile
						end
					end
				end
			else
				if strcmpi(ext, '.root')
					type = 0; % TFile
				elseif strcmpi(ext, '.xml')
					type = 1; % TXMLFile
				end
			end
		else
			error('path:not_found', 'Directory not found or empty filename: %s.', opath);
		end
	end
	
	% compose o- and ifile
	ofile.type = type;
	ofile.path = opath;
	
	sep = filesep;
	
	if type > 1
		ofile.usr = usr;
		ofile.pwd = pwd;
		
		sep = '/';
	end
	
	% split ipath in dirs
	ifile.path = explode(ipath(2:end), sep);
end
