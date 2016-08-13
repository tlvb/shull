pts = load('../data/points.txt');
hes = load('../data/hull_edges.txt');
ies = load('../data/internal_edges.txt');
ccs = load('../data/circumcircles.txt');

clf
hold on;

for i=1:size(ccs)(1)
	plot(ccs(i,1),ccs(i,2), 'g+', 'linewidth', 1.5);
end
for i=1:size(ies)(1)
	plot([ies(i,1) ies(i,3)], [ies(i,2) ies(i, 4)], 'b', 'linewidth', 3);
end
for i=1:size(hes)(1)
	plot([hes(i,1) hes(i,3)], [hes(i,2) hes(i, 4)], 'r', 'linewidth', 3);
end

plot(pts(:,1), pts(:,2), 'kx', 'markersize', 5, 'linewidth', 2);

axis([-10, 110, -10, 110])
axis equal;
hold off;
