# The following has been generated automatically from src/core/symbology/qgsrenderer.h
# monkey patching scoped based enum
QgsFeatureRenderer.Property.HeatmapRadius.__doc__ = "Heatmap renderer radius"
QgsFeatureRenderer.Property.HeatmapMaximum.__doc__ = "Heatmap maximum value"
QgsFeatureRenderer.Property.__doc__ = "Data definable properties for renderers.\n\n.. versionadded:: 3.38\n\n" + '* ``HeatmapRadius``: ' + QgsFeatureRenderer.Property.HeatmapRadius.__doc__ + '\n' + '* ``HeatmapMaximum``: ' + QgsFeatureRenderer.Property.HeatmapMaximum.__doc__
# --
QgsFeatureRenderer.SymbolLevels = QgsFeatureRenderer.Capability.SymbolLevels
QgsFeatureRenderer.MoreSymbolsPerFeature = QgsFeatureRenderer.Capability.MoreSymbolsPerFeature
QgsFeatureRenderer.Filter = QgsFeatureRenderer.Capability.Filter
QgsFeatureRenderer.ScaleDependent = QgsFeatureRenderer.Capability.ScaleDependent
QgsFeatureRenderer.Capabilities = lambda flags=0: QgsFeatureRenderer.Capability(flags)
from enum import Enum


def _force_int(v): return int(v.value) if isinstance(v, Enum) else v


QgsFeatureRenderer.Capability.__bool__ = lambda flag: bool(_force_int(flag))
QgsFeatureRenderer.Capability.__eq__ = lambda flag1, flag2: _force_int(flag1) == _force_int(flag2)
QgsFeatureRenderer.Capability.__and__ = lambda flag1, flag2: _force_int(flag1) & _force_int(flag2)
QgsFeatureRenderer.Capability.__or__ = lambda flag1, flag2: QgsFeatureRenderer.Capability(_force_int(flag1) | _force_int(flag2))
