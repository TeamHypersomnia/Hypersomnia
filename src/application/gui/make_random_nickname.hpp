#pragma once

inline std::string int_to_roman(int num) {
	std::string ans;

	while (num)
	{
		if (num >= 1000)
		{
			ans += "M";
			num -= 1000;
		}
		else if (num >= 900)
		{
			ans += "CM";
			num -= 900;
		}
		else if (num >= 500)
		{
			ans += "D";
			num -= 500;
		}
		else if (num >= 400)
		{
			ans += "CD";
			num -= 400;
		}
		else if (num >= 100)
		{
			ans += "C";
			num -= 100;
		}
		else if (num >= 90)
		{
			ans += "XC";
			num -= 90;
		}
		else if (num >= 50)
		{
			ans += "L";
			num -= 50;
		}
		else if (num >= 40)
		{
			ans += "XL";
			num -= 40;
		}
		else if (num >= 10)
		{
			ans += "X";
			num -= 10;
		}
		else if (num >= 9)
		{
			ans += "IX";
			num -= 9;
		}
		else if (num >= 5)
		{
			ans += "V";
			num -= 5;
		}
		else if (num >= 4)
		{
			ans += "IV";
			num -= 4;
		}
		else
		{
			ans += "I";
			num -= 1;
		}
	}

	return ans;
}
inline std::string make_random_nickname(randomization& rng) {
	auto random_from = [&](auto& cont) {
		return rng.choose_from(cont);
	};

	const std::vector<std::string> surnames = {
		"Aureus", // Golden
		"Ferox", // Fierce
		"Magna", // Great
		"Silens", // Silent
		"Fortis", // Brave
		"Celer", // Swift
		"Prudens", // Prudent
		"Validus", // Strong
		"Acutus", // Sharp
		"Vehemens", // Forceful
		"Clemens", // Merciful
		"Gravis", // Heavy, Important
		"Audax", // Bold
		"Bellator", // Warrior
		"Callidus", // Clever, Skilled
		"Decorus", // Graceful, Handsome
		"Fidelis", // Faithful
		"Gallus", // Gallic
		"Hosti", // Hostile
		"Ingeni", // Ingenious
		"Lucidus", // Bright, Clear
		"Nobilis", // Noble
		"Opacus", // Dark, Shadowy
		"Pernix", // Agile, Quick
		"Quies", // Quiet, Still
		"Rusticus", // Rustic, Rural
		"Severus", // Stern
		"Tutus", // Safe, Secure
		"Vivax", // Lively
		"Vulpinus", // Crafty, Cunning
		"Pius", // Dutiful
		"Victrix", // Conquering
		"Sapiens", // Wise
		"Dux", // Leader, already concise
		"Centurion", // Short for Centurion
		"Imperator", // Short for Imperator
		"Legatus", // Short for Legatus
		"Praefectus", // Short for Praefectus
		"Eques", // Short for Eques
		"Tribunus", // Short for Tribunus
		"Pilus", // Short for Pilus
		"Signifer", // Short for Signifer
		"Optio", // Short for Optio
		"Veles", // Short for Veles
		"Decanus", // Short for Decanus
		"Aquilifer", // Short for Aquilifer
		"Drago", // Short for Drago
		"Gladiator", // Short for Gladiator
		"Sagittarius",
		"Ballista", // Short for Ballistarius
		"Cohortis", // Short for Cohortis
		"Primpilus", // Short for Primipilus
		"Secutor", // Short for Secutor
		"Hastatus", // Short for Hastatus
		"Princeps", // Short for Princeps
		"Triarchus", // Short for Triarchus
		"Navarchus", // Short for Navarchus
		"Hypaspist", // Short for Hypaspist
		"Lochagos", // Short for Lochagos,
		"Caesar"
	};

	const std::vector<std::string> names = {
		"Aurelius", // Golden
		"Cassius", // Vain
		"Decimus", // Tenth
		"Gaius", // To Rejoice
		"Lucius", // Light
		"Marcus", // Warlike
		"Quintus", // Fifth
		"Titus", // Title of Honor
		"Tiberius", // Of the Tiber
		"Octavius", // Eighth
		"Gnaeus", // Unknown Meaning
		"Servius", // Preserve
		"Appius", // Unknown Meaning
		"Publius", // Public
		"Galerius", // Calm
		"Flavius", // Golden
		"Claudius", // Lame
		"Julius", // Downy-Bearded
		"Vitus", // Life
		"Antonius", // Priceless
		"Cornelius", // Horn
		"Felix", // Lucky
		"Marius", // Male
		"Maximus", // Greatest
		"Silvanus", // Forest
		"Tacitus", // Silent
		"Valerius", // Strong
		"Virgil", // Staff Bearer
		"Vespasian", // Wasp
		"Cicero", // Chickpea
		"Hadrian", // From Hadria
		"Sulla", // Unique
		"Numa", // The Kindly One
		"Atticus", // From Attica
		"Scipio", // Staff
		"Pliny", // Unknown Meaning
		"Aemilianus",
		"Augustus",
		"Balbus",
		"Brutus",
		"Cato",
		"Corvinus",
		"Drusus",
		"Fabius",
		"Flaccus",
		"Gracchus",
		"Lentulus",
		"Livius",
		"Lucullus",
		"Marcellus",
		"Nero",
		"Octavianus",
		"Piso",
		"Scaevola",
		"Varro",
		"Aquilinus",
		"Arvina",
		"Asina",
		"Aventinus",
		"Blaesus",
		"Bubulcus",
		"Cincinnatus",
		"Claudian",
		"Cordus",
		"Crassus",
		"Dentatus",
		"Dives",
		"Dolabella",
		"Aphroditus",
		"Fuscus",
		"Galba",
		"Glabrio",
		"Gordianus",
		"Hadrianus",
		"Hirtius",
		"Laevinus",
		"Longinus",
		"Macer",
		"Mancinus",
		"Messala",
		"Nerva",
		"Otho",
		"Paetus",
		"Pompeius",
		"Pontius",
		"Priscus",
		"Quintil",
		"Regulus",
		"Rufinus",
		"Rutilius",
		"Sabinus",
		"Salinator",
		"Scaurus",
		"Sosius",
		"Scribon",
		"Tullius",
		"Ulpius",
		"Urbicus",
		"Valens",
		"Valerian",
		"Vergil",
		"Vespas",
		"Vetus",
		"Virgil",
		"Vitelli",
		"Volesus",
		"Volusianus",
		"Albinius",
		"Albinus",
		"Antonian",
		"Calvinus",
		"Catullus",
		"Geminus",
		"Labeo",
		"Lepidus",
		"Vopiscus"
	};

	return 
		random_from(names) + " " +
		random_from(surnames) + " " +
		int_to_roman(rng.randval(1, 10))
	;
}
