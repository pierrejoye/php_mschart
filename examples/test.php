<?php

$samples = array(
'pie-supplement',
'bar-3d-clustered',
'data-serie-z-order',
'range-indicator'
);
error_reporting(E_ALL|E_NOTICE);
foreach($samples as $sample) {
	$file = __DIR__ . '/' . $sample . '.xml';
	$xml = file_get_contents($file);
	mschart_loadxml($xml, MSCHART_FORMAT_PNG, __DIR__ . "\\" . $sample . '.png');
}
