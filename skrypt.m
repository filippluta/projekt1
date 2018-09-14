if (~exist('s', 'var'))
    s = serial('/dev/ttyUSB1');
    s.BaudRate = 57600;
    fopen(s)
end

t = 0:0.01:10;

traj_x = ceil(sin(t) * 2000);
traj_y = ceil(sin(t + (2/3*pi)) * 2000);
traj_z = ceil(sin(t + (4/3*pi)) * 2000);

kroki_x = traj_x(2:end) - traj_x(1:end-1);
kroki_y = traj_y(2:end) - traj_y(1:end-1);
kroki_z = traj_z(2:end) - traj_z(1:end-1);

% fprintf(s, 'x %d y %d z %d e %d temp %d\n', [0 0 0 5000 190]);
for i = 1:length(kroki_x)
   
    fprintf(s, 'x %d y %d z %d e %d temp %d\n', [kroki_x(i), kroki_y(i), kroki_z(i), 100, 190]);
    while (s.BytesAvailable == 0)
        
    end
    fscanf(s);
end